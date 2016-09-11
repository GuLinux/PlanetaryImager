/*
 * Copyright (C) 2016  Marco Gulino <marco@gulinux.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "qhyccdimager.h"
#include "qhydriver.h"
#include "qhyccd.h"
#include <QDebug>

#include <QThread>
#include "utils.h"
#include <QImage>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>
#include <fps_counter.h>
#include <chrono>
#include "Qt/strings.h"
#include <boost/lockfree/spsc_queue.hpp>


using namespace std;
using namespace std::placeholders;
using namespace std::chrono_literals;


class ImagingWorker : public QObject {
  Q_OBJECT
public:
  ImagingWorker(qhyccd_handle *handle, QHYCCDImager *imager, const ImageHandlerPtr imageHandler, QObject* parent = 0);
public slots:
  void start_live();
  void stop();
  void queue(std::function<void()> f);
private:
  boost::lockfree::spsc_queue<std::function<void()>> jobs_queue;
  qhyccd_handle *handle;
  bool enabled = true;
  QHYCCDImager *imager;
  ImageHandlerPtr imageHandler;
};

class QHYCCDImager::Private {
public:
  Private(const QString& name, const QString& id, const ImageHandlerPtr& imageHandler, QHYCCDImager* q);
  qhyccd_handle *handle;
  QString name;
  QString id;
  ImageHandlerPtr imageHandler;
  Chip chip;
  Controls settings;
  void load_settings();
  QThread imaging_thread;
  ImagingWorker *worker;
  void load(Control &setting);
private:
  QHYCCDImager *q;
};


QHYCCDImager::Private::Private(const QString &name, const QString &id, const ImageHandlerPtr &imageHandler, QHYCCDImager* q) : name(name), id(id), imageHandler{imageHandler}, q{q}
{
}

QHYCCDImager::QHYCCDImager(const QString &cameraName, const char *id, const ImageHandlerPtr &imageHandler) : dptr(cameraName, id, imageHandler, this)
{
  d->handle = OpenQHYCCD(const_cast<char*>(id));
  if(d->handle < QHYCCD_SUCCESS) {
    throw QHYDriver::error("Initializing Camera %1"_q % id, (long)(d->handle));
  }
  if(int result = InitQHYCCD(d->handle) != QHYCCD_SUCCESS) {
    throw QHYDriver::error("Initializing Camera %1"_q % id, result);
  }
  qDebug() << "Camera " << id << "initialized correctly";
  qDebug() << "gain: " << GetQHYCCDParam(d->handle, CONTROL_GAIN) << ", gamma: " << GetQHYCCDParam(d->handle, CONTROL_GAMMA) << ", exposure: " << GetQHYCCDParam(d->handle, CONTROL_EXPOSURE);
  char buf[1024];
  uint8_t st[1024];
  // Not implemented in QHY Library
//   GetQHYCCDCFWStatus(d->handle, buf);
//   qDebug() << "firmware status: " << QByteArray{buf};
  GetQHYCCDFWVersion(d->handle, st);
  copy(begin(st), end(st), begin(buf));
  qDebug() << "firmware version: " << QByteArray{buf};
  double chipwidth, chipheight, pixelwidth, pixelheight;
  uint32_t width, height, bpp;
  GetQHYCCDChipInfo(d->handle, &chipwidth, &chipheight, &width, &height, &pixelwidth, &pixelheight, &bpp);
  d->chip.set_chip_size(chipwidth, chipheight).set_pixel_size(pixelwidth, pixelheight).set_resolution({static_cast<int>(width), static_cast<int>(height)});
  d->chip << Chip::Property{"bpp", bpp};
  qDebug() << d->chip;
  d->load_settings();
  qDebug() << d->settings;
//   GetQHYCCDCameraStatus(d->handle, st); // NOT IMPLEMENTED IN QHY Library
}


QHYCCDImager::~QHYCCDImager()
{
  stopLive();
  qDebug() << "Closing QHYCCD";
  if(d->imaging_thread.isRunning()) {
    d->worker->stop();
    d->imaging_thread.wait();
  }
  int result = CloseQHYCCD(d->handle);
  qDebug() << "CloseQHYCCD result: " << result;
  emit disconnected();
}

QHYCCDImager::Chip QHYCCDImager::chip() const
{
  return d->chip;
}

QString QHYCCDImager::name() const
{
  return d->name;
}

QList< QHYCCDImager::Control > QHYCCDImager::controls() const
{
  return d->settings;
}

void QHYCCDImager::Private::load_settings()
{
  settings.clear();
  for(auto control: QList<QPair<QString,CONTROL_ID>>{
    { "control_brightness", CONTROL_BRIGHTNESS },
    { "control_contrast", CONTROL_CONTRAST },
    { "control_wbr", CONTROL_WBR },
    { "control_wbb", CONTROL_WBB },
    { "control_wbg", CONTROL_WBG },
    { "control_gamma", CONTROL_GAMMA },
    { "control_gain", CONTROL_GAIN },
    { "control_offset", CONTROL_OFFSET },
    { "control_exposure", CONTROL_EXPOSURE },
    { "control_speed", CONTROL_SPEED },
    { "control_transferbit", CONTROL_TRANSFERBIT },
    { "control_channels", CONTROL_CHANNELS },
    { "control_usbtraffic", CONTROL_USBTRAFFIC },
    { "control_rownoisere", CONTROL_ROWNOISERE },
    { "control_curtemp", CONTROL_CURTEMP },
    { "control_curpwm", CONTROL_CURPWM },
    { "control_manulpwm", CONTROL_MANULPWM },
    { "control_cfwport", CONTROL_CFWPORT },
    { "control_cooler", CONTROL_COOLER },
    { "control_st4port", CONTROL_ST4PORT },
    { "cam_color", CAM_COLOR },
    { "cam_bin1x1mode", CAM_BIN1X1MODE },
    { "cam_bin2x2mode", CAM_BIN2X2MODE },
    { "cam_bin3x3mode", CAM_BIN3X3MODE },
    { "cam_bin4x4mode", CAM_BIN4X4MODE },
    { "CAM_MECHANICALSHUTTER", CAM_MECHANICALSHUTTER },
    { "CAM_TRIGER_INTERFACE", CAM_TRIGER_INTERFACE },
    { "CAM_TECOVERPROTECT_INTERFACE", CAM_TECOVERPROTECT_INTERFACE },
    { "CAM_SINGNALCLAMP_INTERFACE", CAM_SINGNALCLAMP_INTERFACE },
    { "CAM_FINETONE_INTERFACE", CAM_FINETONE_INTERFACE },
    { "CAM_SHUTTERMOTORHEATING_INTERFACE", CAM_SHUTTERMOTORHEATING_INTERFACE },
    { "CAM_CALIBRATEFPN_INTERFACE", CAM_CALIBRATEFPN_INTERFACE },
    { "CAM_CHIPTEMPERATURESENSOR_INTERFACE", CAM_CHIPTEMPERATURESENSOR_INTERFACE },
    { "CAM_USBREADOUTSLOWEST_INTERFACE", CAM_USBREADOUTSLOWEST_INTERFACE },

  }) {
    int result = IsQHYCCDControlAvailable(handle, control.second);
//     if(result == QHYCCD_ERROR_NOTSUPPORT) {
//       qDebug() << "control " << control.first << "not supported, skipping";
//       continue;
//     }
    Control setting{control.second, control.first};
    result = GetQHYCCDParamMinMaxStep(handle, control.second, &setting.min, &setting.max, &setting.step);
    if(result != QHYCCD_SUCCESS) {
      qCritical() << "error retrieving control " << control.first << ":" << QHYDriver::error_name(result) << "(" << result << ")";
      continue;
    }
    if(setting.id == CONTROL_GAIN)
      setting.step /= 10.;
    if(setting.id == CONTROL_TRANSFERBIT ) {
      qDebug() << "Changing transferbit setting for " << q->name() << id;
      setting.type = Control::Combo;
      setting.choices = {{"8", 8}, {"16", 16}};
      setting.min = 8;
      setting.max = 16;
    }
    if(setting.id == CONTROL_EXPOSURE) {
      setting.is_duration = true;
      setting.duration_unit = 1us;
    }
    load(setting);
//     setting.value = GetQHYCCDParam(handle, control.second);
    qDebug() << setting;
    settings << setting;
  }
}

void QHYCCDImager::Private::load ( QHYCCDImager::Control& setting )
{
  setting.value = GetQHYCCDParam(handle, static_cast<CONTROL_ID>(setting.id));
}


void QHYCCDImager::setControl(const QHYCCDImager::Control& setting)
{
  d->worker->queue([=]{
    auto result = SetQHYCCDParam(d->handle, static_cast<CONTROL_ID>(setting.id), setting.value);
    if(result != QHYCCD_SUCCESS) {
      qCritical() << "error setting" << setting.name << ":" << QHYDriver::error_name(result) << "(" << result << ")";
      return;
    }
    Control &setting_ref = *find_if(begin(d->settings), end(d->settings), [setting](const Control &s) { return s.id == setting.id; });
    d->load(setting_ref);
    qDebug() << "setting" << setting.name << "updated to value" << setting_ref.value;
    emit changed(setting_ref);
  });
}

void ImagingWorker::queue(std::function< void()> f)
{
  jobs_queue.push(f);
}

void QHYCCDImager::startLive()
{
  d->worker = new ImagingWorker{d->handle, this, d->imageHandler};
  d->worker->moveToThread(&d->imaging_thread);
  connect(&d->imaging_thread, SIGNAL(started()), d->worker, SLOT(start_live()));
  connect(&d->imaging_thread, SIGNAL(finished()), d->worker, SLOT(deleteLater()));
  d->imaging_thread.start();
  qDebug() << "Live started correctly";
}

ImagingWorker::ImagingWorker(qhyccd_handle* handle, QHYCCDImager* imager, const ImageHandlerPtr imageHandler, QObject* parent)
  : QObject(parent), handle(handle), imager{imager}, imageHandler{imageHandler}, jobs_queue(10)
{
}

void ImagingWorker::start_live()
{
  static map<int, Frame::ColorFormat> color_formats {
    {BAYER_GB, Frame::Bayer_GBRG},
    {BAYER_GR, Frame::Bayer_GRBG},
    {BAYER_BG, Frame::Bayer_BGGR},
    {BAYER_RG, Frame::Bayer_RGGB},
  };
  int colorret = IsQHYCCDControlAvailable(handle,CAM_COLOR);
  Frame::ColorFormat color_format;
  if(color_formats.count(colorret)) {
    color_format = color_formats[colorret];
    SetQHYCCDDebayerOnOff(handle, false);
  } else {
    color_format = Frame::Mono;
  }
  
  auto size = GetQHYCCDMemLength(handle);
  qDebug() << "size: " << static_cast<double>(size)/1024. << "kb, required for 8bit: " << (1280.*960.)/1024 << "kb, for 16bit: " << (1280.*960.*2.)/1024. << "kb";
  auto result =  SetQHYCCDStreamMode(handle,1);
  if(result != QHYCCD_SUCCESS) {
    qCritical() << "Unable to set live mode stream";
    return;
  }
  result = BeginQHYCCDLive(handle);
  if(result != QHYCCD_SUCCESS) {
    qCritical() << "Unable to start live mode";
    return;
  }

  fps_counter _fps([=](double rate){ emit imager->fps(rate); }, fps_counter::Elapsed);
  uint8_t buffer[size];
  qDebug() << "capturing thread started, image size: " << size;
  uint32_t w, h, bpp, channels;
  auto buffer_real_size = [&] { return w*h*channels*(bpp<=8?1:2); };
  auto is_zero = [](uint8_t v) { return v==0; };
  while(enabled){
    std::function<void()> f;
    while(jobs_queue.pop(f))
      f();
    result = GetQHYCCDLiveFrame(handle,&w,&h,&bpp,&channels,buffer);
    if(result != QHYCCD_SUCCESS || all_of(buffer, &buffer[buffer_real_size()], is_zero )) {
      qWarning() << "Error capturing live frame: " << result;
//       QThread::msleep(1);
    } else {
      int type = bpp==8 ? CV_8UC1 : CV_16UC1;
      if(channels == 3)
        type = bpp==8 ? CV_8UC3 : CV_16UC3;
      cv::Mat image({static_cast<int>(w), static_cast<int>(h)}, type, buffer);
      ++_fps;
      cv::Mat copy;
      image.copyTo(copy);
      imageHandler->handle(Frame::create(copy, color_format)); // TODO: Properly handle with debayer setting, I guess... find a tester!
    }
  }
  result = StopQHYCCDLive(handle);
  qDebug() << "Stop live capture result: " << result;
  QThread::currentThread()->quit();
}


void ImagingWorker::stop()
{
  enabled = false;
}


void QHYCCDImager::stopLive()
{
  d->worker->stop();
}

bool QHYCCDImager::supportsROI()
{
  return false; // TODO
}

void QHYCCDImager::setROI(const QRect&)
{

}

void QHYCCDImager::clearROI()
{

}


#include "qhyccdimager.moc"

