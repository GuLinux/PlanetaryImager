/*
 * Copyright (C) 2016 Marco Gulino (marco AT gulinux.net)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include "zwo_asi_imager.h"
#include <stringbuilder.h>
#include <QObject>
#include <QThread>
#include <fps_counter.h>
#include <QRect>
#include <atomic>
#include "Qt/strings.h"
using namespace std;
using namespace GuLinux;


namespace {
const int64_t ImageTypeSettingId = 10000;
const int64_t BinSettingId = 10001;
class ImagingWorker : public QObject {
#ifndef IN_IDE_PARSER
    Q_OBJECT
#endif
public:
    ImagingWorker(const ASI_CAMERA_INFO &info, const ImageHandlerPtr &imageHandler, ZWO_ASI_Imager *imager);
    ~ImagingWorker();
    atomic_bool stop;
public slots:
    void start_live();
    void stop_live();
    void setFormat(ASI_IMG_TYPE format);
    void setBin(int bin);
    void setROI(const QRect &roi);
    QRect maxROI() const;
    QRect supportedROI(const QRect &rawROI) const;
private:
    ASI_CAMERA_INFO info;
    ImageHandlerPtr imageHandler;
    ZWO_ASI_Imager *imager;
    ASI_IMG_TYPE imageFormat;
    int bin;
    QRect roi;
    struct RestartShooting {
        RestartShooting(ImagingWorker *q) : q(q) {
            q->stop_live();
        }
        ~RestartShooting() {
            q->start_live();
        }
        ImagingWorker *q;
    };
    size_t calcBufferSize();
    int getCVImageType();
};
}


DPTR_IMPL(ZWO_ASI_Imager) {
    ASI_CAMERA_INFO info;
    ImageHandlerPtr imageHandler;
    ZWO_ASI_Imager *q;

    Chip chip;
    ImagingWorker *worker = nullptr;
    QThread imaging_thread;
    ASI_IMG_TYPE currentFormat;
    Imager::Setting setting(ASI_CONTROL_TYPE settingId);
};

ZWO_ASI_Imager::ZWO_ASI_Imager(const ASI_CAMERA_INFO &info, const ImageHandlerPtr &imageHandler) : dptr(info, imageHandler, this)
{
    d->chip.xres = info.MaxWidth;
    d->chip.yres = info.MaxHeight;
    d->chip.pixelheight = info.PixelSize;
    d->chip.pixelwidth = info.PixelSize;
    d->chip.height = info.MaxHeight * info.PixelSize / 1000;
    d->chip.width = info.MaxWidth * info.PixelSize / 1000;
    d->chip.properties.push_back( {"Camera Speed", info.IsUSB3Camera ? "USB3" : "USB2"});
    d->chip.properties.push_back( {"Host Speed", info.IsUSB3Host ? "USB3" : "USB2"});
    int result = ASIOpenCamera(info.CameraID);
    if(result != ASI_SUCCESS) {
        throw runtime_error(stringbuilder() << "Error opening camera: " << result );
    }
}

ZWO_ASI_Imager::~ZWO_ASI_Imager()
{
    ASICloseCamera(d->info.CameraID);
}

Imager::Chip ZWO_ASI_Imager::chip() const
{
    return d->chip;
}

QString ZWO_ASI_Imager::name() const
{
    return d->info.Name;
}

void ZWO_ASI_Imager::setSetting(const Setting& setting)
{
    qDebug() << __PRETTY_FUNCTION__;
    if(setting.id == ImageTypeSettingId) {
        d->worker->stop = true;
        QMetaObject::invokeMethod(d->worker, "setFormat", Q_ARG(ASI_IMG_TYPE, static_cast<ASI_IMG_TYPE>(setting.value) ) );
        return;
    }
    if(setting.id == BinSettingId) {
      d->worker->stop = true;
      QMetaObject::invokeMethod(d->worker, "setBin", Q_ARG(int, static_cast<int>(setting.value) ) );
      return;
    }

    ASISetControlValue(d->info.CameraID, static_cast<ASI_CONTROL_TYPE>(setting.id), setting.value, ASI_FALSE);
    emit changed(d->setting(static_cast<ASI_CONTROL_TYPE>(setting.id)));

}

Imager::Settings ZWO_ASI_Imager::settings() const
{
    qDebug() << __PRETTY_FUNCTION__;
    list<ASI_CONTROL_TYPE> settings_enum {
        ASI_GAIN,
        ASI_EXPOSURE,
        ASI_GAMMA,
        ASI_WB_R,
        ASI_WB_B,
        ASI_BRIGHTNESS,
        ASI_BANDWIDTHOVERLOAD,
        ASI_OVERCLOCK,
        ASI_TEMPERATURE,// return 10*temperature
        ASI_FLIP,
        ASI_AUTO_MAX_GAIN,
        ASI_AUTO_MAX_EXP,
        ASI_AUTO_MAX_BRIGHTNESS,
        ASI_HARDWARE_BIN,
        ASI_HIGH_SPEED_MODE,
        ASI_COOLER_POWER_PERC,
        ASI_TARGET_TEMP,// not need *10
        ASI_COOLER_ON,
        ASI_MONO_BIN//lead to less grid at software bin mode for color camera
    };
    Imager::Settings settings;
    for(auto v: settings_enum) {
        auto setting = d->setting(v);
        if(setting != Imager::Setting {} )
            settings.push_back(setting);
    }
    static map<ASI_IMG_TYPE, QString> format_names {
        {ASI_IMG_RAW8, "Raw 8bit"},
        {ASI_IMG_RGB24, "RGB24"},
        {ASI_IMG_RAW16, "RAW 16bit"},
        {ASI_IMG_Y8, "Y8"},
    };
    Imager::Setting imageFormat {ImageTypeSettingId, "Image Format", 0, 0, 1, d->currentFormat, 0, Setting::Combo};
    int i = 0;
    while(d->info.SupportedVideoFormat[i] != ASI_IMG_END && i < 8) {
        auto format = d->info.SupportedVideoFormat[i];
        qDebug() << "supported format: " << format << ": " << format_names[format];
        imageFormat.choices.push_back( {format_names[format], format});
        ++i;
    }
    imageFormat.max = i-1;
    settings.push_back(imageFormat);
    
    Imager::Setting bin {BinSettingId, "Bin", 0, 0, 1, 1, 1, Setting::Combo};
    i = 0;
    while(d->info.SupportedBins[i] != 0) {
      auto bin_value = d->info.SupportedBins[i++];
      bin.choices.push_back( {"%1x%1"_q % bin_value, bin_value } );
    }
    bin.max = i-1;
    settings.push_back(bin);
    return settings;
}

Imager::Setting ZWO_ASI_Imager::Private::setting(ASI_CONTROL_TYPE settingId)
{
    ASI_CONTROL_CAPS caps;
    int result = ASIGetControlCaps(info.CameraID, settingId, &caps);
    if(result != ASI_SUCCESS) {
        qDebug() << "error retrieving setting " << settingId << ": " << result;
        return {};
    }
    long value;
    ASI_BOOL isAuto;
    ASIGetControlValue(info.CameraID, settingId, &value, &isAuto);
    return {caps.ControlType, caps.Description, caps.MinValue, caps.MaxValue, 1, value, caps.DefaultValue, Imager::Setting::Number};
}

void ZWO_ASI_Imager::startLive()
{
    qDebug() << __PRETTY_FUNCTION__;
    d->worker = new ImagingWorker(d->info, d->imageHandler, this);
    d->worker->moveToThread(&d->imaging_thread);
    connect(&d->imaging_thread, SIGNAL(started()), d->worker, SLOT(start_live()));
    connect(&d->imaging_thread, SIGNAL(finished()), d->worker, SLOT(deleteLater()));
    d->imaging_thread.start();
    qDebug() << "Live started correctly";
}

void ZWO_ASI_Imager::stopLive()
{
    qDebug() << __PRETTY_FUNCTION__;

}

ImagingWorker::ImagingWorker(const ASI_CAMERA_INFO& info, const ImageHandlerPtr &imageHandler, ZWO_ASI_Imager *imager)
    : info(info),
      imageHandler {imageHandler},
               imager {imager},
               imageFormat {info.SupportedVideoFormat[0]},
               bin {info.SupportedBins[0]},
roi {0, 0, info.MaxWidth, info.MaxHeight}
{
}

ImagingWorker::~ImagingWorker()
{
}


void ImagingWorker::start_live()
{
    qDebug() << "Starting imaging: imageFormat=" << imageFormat << ", roi: " << roi << ", bin: " << bin;
    QRect roi = supportedROI(this->roi);
    int result = ASISetROIFormat(info.CameraID, roi.width(), roi.height(), bin, imageFormat);
    if(result != ASI_SUCCESS)
        throw runtime_error(stringbuilder() << "Error setting format: " << result );
    result = ASISetStartPos(info.CameraID, roi.x(), roi.y());
    if(result != ASI_SUCCESS)
        throw runtime_error(stringbuilder() << "Error setting ROI position: " << result );
    result = ASIStartVideoCapture(info.CameraID);
    if(result != ASI_SUCCESS)
        throw runtime_error(stringbuilder() << "Error starting capture: " << result );
    stop = false;
    vector<uint8_t> buffer(calcBufferSize());
    fps_counter _fps([=](double rate) {
        emit imager->fps(rate);
    }, fps_counter::Elapsed);
    qDebug() << "Imaging started: imageFormat=" << imageFormat << ", roi: " << roi << ", bin: " << bin;
    while(!stop) {
        result = ASIGetVideoData(info.CameraID, buffer.data(), buffer.size(), 100000);
        if(result == ASI_SUCCESS) {
            cv::Mat image( {roi.width(), roi.height()}, getCVImageType(), buffer.data());
            cv::Mat copy;
            image.copyTo(copy);
            imageHandler->handle(copy);
            ++_fps;
        } else {
            qDebug() << "Capture error: " << result;
        }
    }
}
size_t ImagingWorker::calcBufferSize()
{
    auto base_size = roi.width() * roi.height();
    switch(imageFormat) {
      case ASI_IMG_RAW8:
	return base_size;
      case ASI_IMG_RAW16:
	return base_size * 2;
      case ASI_IMG_RGB24:
	return base_size * 3;
      default:
	throw runtime_error("Format not supported");
    }
}

int ImagingWorker::getCVImageType()
{
  switch(imageFormat) {
    case ASI_IMG_RAW8:
      return CV_8UC1;
    case ASI_IMG_RAW16:
      return CV_16UC1;
    case ASI_IMG_RGB24:
      return CV_8UC3;
    default:
      throw runtime_error("Format not supported");
      
  }
}

void ImagingWorker::stop_live()
{
    stop = true;
    ASIStopVideoCapture(info.CameraID);
    qDebug() << "Imaging stopped.";
}



void ImagingWorker::setBin(int bin)
{
    RestartShooting r(this);
    this->bin = bin;
    this->roi = maxROI();
}

void ImagingWorker::setFormat(ASI_IMG_TYPE format)
{
    RestartShooting r(this);
    this->imageFormat = format;
}

void ImagingWorker::setROI(const QRect& roi)
{
    RestartShooting r(this);
    this->roi = roi;
}

bool ZWO_ASI_Imager::supportsROI()
{
  return true; // TODO: detection?
}

void ZWO_ASI_Imager::clearROI()
{
    d->worker->stop = true;
    QMetaObject::invokeMethod(d->worker, "setROI", Q_ARG(QRect, d->worker->maxROI() ) );
}

void ZWO_ASI_Imager::setROI(const QRect& roi)
{
    d->worker->stop = true;
    QMetaObject::invokeMethod(d->worker, "setROI", Q_ARG(QRect, roi) );
}

QRect ImagingWorker::maxROI() const
{
  return {0, 0, info.MaxWidth / bin, info.MaxHeight / bin};
}

QRect ImagingWorker::supportedROI(const QRect& rawROI) const
{
  return {rawROI.x(), rawROI.y(), (rawROI.width() / 4) * 4, (rawROI.height()/4) * 4 };
}


#include "zwo_asi_imager.moc"
