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
#include "Qt/strings.h"
using namespace std;
using namespace GuLinux;


namespace {
  const int64_t ImageTypeSettingId = 10000;
  class ImagingWorker : public QObject {
    Q_OBJECT
  public:
    ImagingWorker(const ASI_CAMERA_INFO &info, const ImageHandlerPtr &imageHandler, ZWO_ASI_Imager *imager);
    ~ImagingWorker();
  public slots:
    void start_live();
    void stop_live();
  private:
    ASI_CAMERA_INFO info;
    bool stop;
    ImageHandlerPtr imageHandler;
    ZWO_ASI_Imager *imager;
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
};

ZWO_ASI_Imager::ZWO_ASI_Imager(const ASI_CAMERA_INFO &info, const ImageHandlerPtr &imageHandler) : dptr(info, imageHandler, this)
{
  d->chip.xres = info.MaxWidth;
  d->chip.yres = info.MaxHeight;
  d->chip.pixelheight = info.PixelSize;
  d->chip.pixelwidth = info.PixelSize;
  d->chip.height = info.MaxHeight * info.PixelSize / 1000;
  d->chip.width = info.MaxWidth * info.PixelSize / 1000;
  d->chip.properties.push_back({"Camera Speed", info.IsUSB3Camera ? "USB3" : "USB2"});
  d->chip.properties.push_back({"Host Speed", info.IsUSB3Host ? "USB3" : "USB2"});
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
    return;
  }
  
  ASISetControlValue(d->info.CameraID, static_cast<ASI_CONTROL_TYPE>(setting.id), setting.value, ASI_FALSE);

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
    ASI_CONTROL_CAPS caps;
    int result = ASIGetControlCaps(d->info.CameraID, v, &caps);
    if(result != ASI_SUCCESS) {
      qDebug() << "error retrieving setting " << v << ": " << result;
      break;
    }
    long value;
    ASI_BOOL isAuto;
    ASIGetControlValue(d->info.CameraID, v, &value, &isAuto);
    Imager::Setting setting{caps.ControlType, "%1\n%2"_q % caps.Name % caps.Description, caps.MinValue, caps.MaxValue, 1, value, caps.DefaultValue, Imager::Setting::Number};
    settings.push_back(setting);
  }
  static map<ASI_IMG_TYPE, QString> format_names {
    {ASI_IMG_RAW8, "Raw 8bit"},
    {ASI_IMG_RGB24, "RGB24"},
    {ASI_IMG_RAW16, "RAW 16bit"},
    {ASI_IMG_Y8, "Y8"},
  };
  Imager::Setting imageFormat{ImageTypeSettingId, "Image Format", 0, 0, 1, d->currentFormat, 0, Setting::Combo};
  int i = 0;
  while(d->info.SupportedVideoFormat[i] != ASI_IMG_END && i < 8) {
    auto format = d->info.SupportedVideoFormat[i];
    qDebug() << "supported format: " << format << ": " << format_names[format];
    imageFormat.choices.push_back({format_names[format], format});
    ++i;
  }
  imageFormat.max = i-1;
  settings.push_back(imageFormat);
  qDebug() << imageFormat;
  return settings;
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

ImagingWorker::ImagingWorker(const ASI_CAMERA_INFO& info, const ImageHandlerPtr &imageHandler, ZWO_ASI_Imager *imager) : info(info), imageHandler{imageHandler}, imager{imager}
{
}

ImagingWorker::~ImagingWorker()
{
}


void ImagingWorker::start_live()
{
  int result = ASISetROIFormat(info.CameraID, info.MaxWidth, info.MaxHeight, 1, ASI_IMG_RAW8);
  if(result != ASI_SUCCESS) 
    throw runtime_error(stringbuilder() << "Error setting format: " << result );
  result = ASIStartVideoCapture(info.CameraID);
  if(result != ASI_SUCCESS) 
    throw runtime_error(stringbuilder() << "Error starting capture: " << result );
  stop = false;
  vector<uint8_t> buffer(info.MaxHeight*info.MaxWidth);
  fps_counter _fps([=](double rate){ emit imager->fps(rate); }, fps_counter::Elapsed);

  while(!stop) {
    result = ASIGetVideoData(info.CameraID, buffer.data(), buffer.size(), 100000);
    if(result == ASI_SUCCESS) {
    cv::Mat image({info.MaxWidth, info.MaxHeight}, CV_8UC1, buffer.data());
    cv::Mat copy;
    image.copyTo(copy);
    imageHandler->handle(copy);
      ++_fps;
    } else {
      qDebug() << "Capture error: " << result;
    }
  }
}

void ImagingWorker::stop_live()
{

}




#include "zwo_asi_imager.moc"