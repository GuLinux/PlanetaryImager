/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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
#include "image_data.h"
#include <fps_counter.h>
#include "Qt/strings.h"


using namespace std;
using namespace std::placeholders;


class ImagingWorker : public QObject {
  Q_OBJECT
public:
  ImagingWorker(qhyccd_handle *handle, QHYCCDImager *imager, const ImageHandlerPtr imageHandler, QObject* parent = 0);
public slots:
  void start_live();
  void stop();
private:
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
  Settings settings;
  void load_settings();
  QThread imaging_thread;
  ImagingWorker *worker;
  void load(Setting &setting);
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
  GetQHYCCDChipInfo(d->handle, &d->chip.width, &d->chip.height, &d->chip.xres, &d->chip.yres, &d->chip.pixelwidth, &d->chip.pixelheight, &d->chip.bpp);
  qDebug() << d->chip;
  d->load_settings();
  qDebug() << d->settings;
//   GetQHYCCDCameraStatus(d->handle, st); // NOT IMPLEMENTED IN QHY Library
}


QHYCCDImager::~QHYCCDImager()
{
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

QList< QHYCCDImager::Setting > QHYCCDImager::settings() const
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
    Setting setting{control.second, control.first};
    result = GetQHYCCDParamMinMaxStep(handle, control.second, &setting.min, &setting.max, &setting.step);
    if(result != QHYCCD_SUCCESS) {
      qCritical() << "error retrieving control " << control.first << ":" << QHYDriver::error_name(result) << "(" << result << ")";
      continue;
    }
    if(setting.id == CONTROL_GAIN)
      setting.step /= 10;
    load(setting);
//     setting.value = GetQHYCCDParam(handle, control.second);
    qDebug() << setting;
    settings << setting;
  }
}

void QHYCCDImager::Private::load ( QHYCCDImager::Setting& setting )
{
  setting.value = GetQHYCCDParam(handle, static_cast<CONTROL_ID>(setting.id));
}


void QHYCCDImager::setSetting(const QHYCCDImager::Setting& setting)
{
  auto result = SetQHYCCDParam(d->handle, static_cast<CONTROL_ID>(setting.id), setting.value);
  if(result != QHYCCD_SUCCESS) {
    qCritical() << "error setting" << setting.name << ":" << QHYDriver::error_name(result) << "(" << result << ")";
    return;
  }
  Setting &setting_ref = *find_if(begin(d->settings), end(d->settings), [setting](const Setting &s) { return s.id == setting.id; });
  d->load(setting_ref);
  emit changed(setting_ref);
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

ImagingWorker::ImagingWorker(qhyccd_handle* handle, QHYCCDImager* imager, const ImageHandlerPtr imageHandler, QObject* parent): QObject(parent), handle(handle), imager{imager}, imageHandler{imageHandler}
{
}

void ImagingWorker::start_live()
{
  auto size = GetQHYCCDMemLength(handle);
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
  int w, h, bpp, channels;
  auto buffer_real_size = [&] { return w*h*channels*(bpp<=8?1:2); };
  auto is_zero = [](uint8_t v) { return v==0; };
  while(enabled){
    result = GetQHYCCDLiveFrame(handle,&w,&h,&bpp,&channels,buffer);
    if(result != QHYCCD_SUCCESS || all_of(buffer, &buffer[buffer_real_size()], is_zero )) {
      qWarning() << "Error capturing live frame: " << result;
//       QThread::msleep(1);
    } else {
      int type = bpp==8 ? CV_8UC1 : CV_16UC1;
      if(channels == 3)
        type = bpp==8 ? CV_8UC3 : CV_16UC3;
      cv::Mat image({w, h}, type, buffer);
      ++_fps;
      cv::Mat copy;
      image.copyTo(copy);
      imageHandler->handle(copy);
      cv::VideoCapture cap;
      cap.
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


#include "qhyccdimager.moc"

