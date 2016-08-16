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
#include "drivers/imagerthread.h"
#include <stringbuilder.h>
#include <QObject>
#include <QThread>
#include <fps_counter.h>
#include <QRect>
#include <atomic>
#include "Qt/strings.h"
#include "utils.h"
#include "zwoexception.h"

using namespace std;
using namespace GuLinux;


namespace {
const int64_t ImageTypeSettingId = 10000;
const int64_t BinSettingId = 10001;
}




DPTR_IMPL(ZWO_ASI_Imager) {
    class Worker : public ImagerThread::Worker {
    public:
        Worker(const QRect &roi, int bin, const ASI_CAMERA_INFO &info, ASI_IMG_TYPE format);
        size_t calcBufferSize();
        virtual bool shoot(const ImageHandlerPtr& imageHandler);
        virtual void start();
        virtual void stop();
        int getCVImageType();
        vector<uint8_t> buffer;
        QRect roi;
        int bin;
        ASI_IMG_TYPE format;
        ASI_CAMERA_INFO info;
    };
    ASI_CAMERA_INFO info;
    ImageHandlerPtr imageHandler;
    ZWO_ASI_Imager *q;

    Chip chip;

    Imager::Setting setting(int settingId);
    Imager::Settings settings;
    
    ImagerThread::ptr imager_thread;
    shared_ptr<Worker> worker;
    QRect maxROI(int bin) const;
    void start_thread(int bin, const QRect& roi, ASI_IMG_TYPE format);
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
    ASI_CHECK << ASIOpenCamera(info.CameraID) << "Open Camera";
}

ZWO_ASI_Imager::~ZWO_ASI_Imager()
{
    stopLive();
    ASI_CHECK << ASICloseCamera(d->info.CameraID) << "Close Camera";
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
        d->start_thread(d->worker->bin, d->worker->roi, static_cast<ASI_IMG_TYPE>(setting.value));
        return;
    }
    if(setting.id == BinSettingId) {
        auto bin = static_cast<int>(setting.value);
        d->start_thread(bin, d->maxROI(bin), d->worker->format);
        return;
    }
    
    size_t index = find_if(d->settings.begin(), d->settings.end(), [&](const Setting &s) { return s.id == setting.id; } ) - d->settings.begin();
    qDebug() << "Changing setting " << index << ": " << d->settings[index];
    int result = ASISetControlValue(d->info.CameraID, static_cast<ASI_CONTROL_TYPE>(setting.id), static_cast<long>(setting.value), ASI_FALSE);
    //if(result =! ASI_SUCCESS)
    //  throw runtime_error(stringbuilder() << "Error setting caps" << setting.id << " to " << setting.value << ": " << result);
    
    d->settings[index] = d->setting(index);
    qDebug() << "Changed setting " << index << ": " << d->settings[index];
    emit changed(d->settings[index]);
}

void ZWO_ASI_Imager::Private::start_thread(int bin, const QRect& roi, ASI_IMG_TYPE format)
{
    worker.reset();
    imager_thread.reset();
    worker = make_shared<Worker>(roi, bin, info, format);
    imager_thread = make_shared<ImagerThread>(worker, q, imageHandler);
    imager_thread->start();
}


Imager::Settings ZWO_ASI_Imager::settings() const
{
    qDebug() << __PRETTY_FUNCTION__;
    int settings_number;
    int result = ASIGetNumOfControls(d->info.CameraID, &settings_number);
    if(result != ASI_SUCCESS)
      throw runtime_error(stringbuilder() << "Error retrieving settings number: " << result);

    d->settings.clear();
    for(int setting_index = 0; setting_index < settings_number; setting_index++) {
        auto setting = d->setting(setting_index);
        if(setting != Imager::Setting {} )
            d->settings.push_back(setting);
    }
    static map<ASI_IMG_TYPE, QString> format_names {
        {ASI_IMG_RAW8, "Raw 8bit"},
        {ASI_IMG_RGB24, "RGB24"},
        {ASI_IMG_RAW16, "RAW 16bit"},
        {ASI_IMG_Y8, "Y8"},
    };
    Imager::Setting imageFormat {ImageTypeSettingId, "Image Format", 0, 0, 1, d->worker->format, 0, Setting::Combo};
    int i = 0;
    while(d->info.SupportedVideoFormat[i] != ASI_IMG_END && i < 8) {
        auto format = d->info.SupportedVideoFormat[i];
        qDebug() << "supported format: " << format << ": " << format_names[format];
        imageFormat.choices.push_back( {format_names[format], format});
        ++i;
    }
    imageFormat.max = i-1;
    d->settings.push_back(imageFormat);

    Imager::Setting bin {BinSettingId, "Bin", 0, 0, 1, 1, 1, Setting::Combo};
    i = 0;
    while(d->info.SupportedBins[i] != 0) {
        auto bin_value = d->info.SupportedBins[i++];
        bin.choices.push_back( {"%1x%1"_q % bin_value, bin_value } );
    }
    bin.max = i-1;
    d->settings.push_back(bin);
    return d->settings;
}

Imager::Setting ZWO_ASI_Imager::Private::setting(int settingIndex)
{
    ASI_CONTROL_CAPS caps;
    int result = ASIGetControlCaps(info.CameraID, settingIndex, &caps);
    if(result != ASI_SUCCESS) {
        qDebug() << "error retrieving setting " << settingIndex << ": " << result;
        return {};
    }
    long value;
    ASI_BOOL isAuto;
    ASIGetControlValue(info.CameraID, caps.ControlType, &value, &isAuto);
    return {caps.ControlType, caps.Description, caps.MinValue, caps.MaxValue, 1, value, caps.DefaultValue, Imager::Setting::Number};
}

void ZWO_ASI_Imager::startLive()
{
    LOG_F_SCOPE
    d->start_thread(1, d->maxROI(1), d->info.SupportedVideoFormat[0]);
    qDebug() << "Live started correctly";
}

void ZWO_ASI_Imager::stopLive()
{
    d->imager_thread.reset();
}


ZWO_ASI_Imager::Private::Worker::Worker(const QRect& requestedROI, int bin, const ASI_CAMERA_INFO& info, ASI_IMG_TYPE format)
    : format {format}, info {info}, bin {bin}, roi {requestedROI.x(), requestedROI.y(), (requestedROI.width() / 4) * 4, (requestedROI.height()/4) * 4 }
{
    qDebug() << "Starting imaging: imageFormat=" << format << ", roi: " << roi << ", bin: " << bin;
    ASI_CHECK << ASISetROIFormat(info.CameraID, roi.width(), roi.height(), bin, format) << "Set format";
    ASI_CHECK << ASISetStartPos(info.CameraID, roi.x(), roi.y()) << "Set ROI position";
    ASI_CHECK << ASIStartVideoCapture(info.CameraID) << "Start video capture";
    buffer.resize(calcBufferSize());
    qDebug() << "Imaging started: imageFormat=" << format << ", roi: " << roi << ", bin: " << bin;
}

void ZWO_ASI_Imager::Private::Worker::start()
{
}

bool ZWO_ASI_Imager::Private::Worker::shoot(const ImageHandlerPtr& imageHandler)
{
  try {
    ASI_CHECK << ASIGetVideoData(info.CameraID, buffer.data(), buffer.size(), 100000) << "Capture frame";
        cv::Mat image( {roi.width(), roi.height()}, getCVImageType(), buffer.data());
        cv::Mat copy;
        image.copyTo(copy);
        imageHandler->handle(copy);
        return true;
  }
   catch(ZWOException &e) {
      qDebug() << QString::fromStdString(e.what());
      return false;
  }
}

void ZWO_ASI_Imager::Private::Worker::stop()
{
    ASI_CHECK << ASIStopVideoCapture(info.CameraID) << "Stop capture";
    qDebug() << "Imaging stopped.";
}

size_t ZWO_ASI_Imager::Private::Worker::calcBufferSize()
{
    auto base_size = roi.width() * roi.height();
    switch(format) {
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

int ZWO_ASI_Imager::Private::Worker::getCVImageType()
{
    switch(format) {
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

bool ZWO_ASI_Imager::supportsROI()
{
    return true; // TODO: detection?
}

void ZWO_ASI_Imager::clearROI()
{
    d->start_thread(d->worker->bin, d->maxROI(d->worker->bin), d->worker->format);
}

void ZWO_ASI_Imager::setROI(const QRect& roi)
{
    d->start_thread(d->worker->bin, roi, d->worker->format);
}

QRect ZWO_ASI_Imager::Private::maxROI(int bin) const
{
    return {0, 0, info.MaxWidth / bin, info.MaxHeight / bin};
}


#include "zwo_asi_imager.moc"
