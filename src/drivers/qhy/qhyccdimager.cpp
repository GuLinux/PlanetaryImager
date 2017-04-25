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
#include "commons/utils.h"
#include <QImage>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>
#include "commons/fps_counter.h"
#include <chrono>
#include "Qt/strings.h"
#include "qhyexception.h"
#include "qhyimagingworker.h"
#include <chrono>
#include "c++/stlutils.h"
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace std::placeholders;
using namespace std::chrono_literals;

DPTR_IMPL(QHYCCDImager) {
  QString name;
  QString id;
  QHYCCDImager *q;
  qhyccd_handle *handle;
    Properties chip;
  Controls controls;
  QHYImagingWorker::ptr imaging_worker;
  void load_controls();
  void load(Control &setting);
};


QHYCCDImager::QHYCCDImager(const QString &cameraName, const char *id, const ImageHandler::ptr &imageHandler) : Imager{imageHandler}, dptr(cameraName, id, this)
{
  qDebug() << "Opening QHY camera " << cameraName << ", id=" << id;
  d->handle = OpenQHYCCD(const_cast<char*>(id));
  if(d->handle == nullptr) {
    QHY_CHECK << QHYCCD_ERROR << "Opening Camera %1"_q % cameraName;
  }
  qDebug() << "initializing camera...";
  QHY_CHECK << InitQHYCCD(d->handle) << "Initializing Camera %1"_q % cameraName;
  qDebug() << "camera initialized";
  uint8_t buffer[1024];
  // Not implemented in QHY Library
//   GetQHYCCDCFWStatus(d->handle, buf);
//   qDebug() << "firmware status: " << QByteArray{buf};
  QHY_CHECK << GetQHYCCDFWVersion(d->handle, buffer) << "Getting firmware version";
  qDebug() << "firmware version: " << QString::fromLocal8Bit(reinterpret_cast<const char*>(buffer));
  double chipwidth, chipheight, pixelwidth, pixelheight;
  uint32_t width, height, bpp;
  QHY_CHECK << GetQHYCCDChipInfo(d->handle, &chipwidth, &chipheight, &width, &height, &pixelwidth, &pixelheight, &bpp) << "Getting chip information for %1" << id;
  d->chip.set_chip_size(chipwidth, chipheight).set_pixel_size(pixelwidth, pixelheight).set_resolution({static_cast<int>(width), static_cast<int>(height)});
  d->chip << Properties::Property{"bpp", bpp};
  d->chip << LiveStream;
  qDebug() << d->chip;
  d->load_controls();
  qDebug() << d->controls;
  qDebug() << "Finished initializing QHY camera" << d->name;
//   GetQHYCCDCameraStatus(d->handle, st); // NOT IMPLEMENTED IN QHY Library
}


QHYCCDImager::~QHYCCDImager()
{
  qDebug() << "Closing QHYCCD";
  QHY_CHECK << CloseQHYCCD(d->handle) << "CloseQHYCCD result: ";
  emit disconnected();
}

QHYCCDImager::Properties QHYCCDImager::properties() const
{
  return d->chip;
}

QString QHYCCDImager::name() const
{
  return d->name;
}

QList< QHYCCDImager::Control > QHYCCDImager::controls() const
{
  return d->controls;
}

void QHYCCDImager::Private::load_controls()
{
  controls.clear();
  static QList<QPair<QString,CONTROL_ID>>qhy_controls{
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
  };
  for(auto qhy_control: qhy_controls) {
    try {
      QHY_CHECK << IsQHYCCDControlAvailable(handle, qhy_control.second) << GuLinux::stringbuilder() << "checking control availability for control " << qhy_control.second;
      double min, max, step;
      QHY_CHECK << GetQHYCCDParamMinMaxStep(handle, qhy_control.second, &min, &max, &step) << GuLinux::stringbuilder() << "checking control range for control " << qhy_control.second;
      
      if(qhy_control.second == CONTROL_GAIN)
        step /= 10.;
      auto control = Control{qhy_control.second, qhy_control.first}.set_range(min, max, step);
      if(control.id == CONTROL_TRANSFERBIT ) {
        qDebug() << "Changing transferbit setting for " << q->name() << id;
                control.type = Control::Combo;
                control.add_choice("8", 8).add_choice("16", 16);
      }
      if(control.id == CONTROL_EXPOSURE) {
                control.is_duration = true;
                control.duration_unit = 1us;
                control.set_is_exposure(true);
      }
      load(control);
  //     setting.value = GetQHYCCDParam(handle, control.second);
      qDebug() << control;
            controls << control;
    } catch(const QHYException &e) {
      qWarning() << e.what();
    }
  }
}

void QHYCCDImager::Private::load ( QHYCCDImager::Control& setting )
{
  setting.value = GetQHYCCDParam(handle, static_cast<CONTROL_ID>(setting.id));
}


void QHYCCDImager::setControl(const QHYCCDImager::Control& setting)
{
  wait_for(push_job_on_thread([=]{
    QHY_CHECK << SetQHYCCDParam(d->handle, static_cast<CONTROL_ID>(setting.id), setting.value.toDouble()) << "Setting control " << setting.name << " to value " << setting.value.toDouble();
    Control &setting_ref = *find_if(begin(d->controls), end(d->controls), [setting](const Control &s) { return s.id == setting.id; });
    d->load(setting_ref);
    qDebug() << "setting" << setting.name << "updated to value" << setting_ref.value;
    emit changed(setting_ref);
  }));
}


void QHYCCDImager::startLive()
{
  restart([=] { return d->imaging_worker = make_shared<QHYImagingWorker>(d->handle); });
}

void QHYCCDImager::setROI(const QRect&)
{

}

void QHYCCDImager::clearROI()
{

}


#include "qhyccdimager.moc"

