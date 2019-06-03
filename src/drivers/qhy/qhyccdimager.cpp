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

#ifndef _WIN32
#include <libusb.h>
#endif


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
#include "Qt/qt_strings_helper.h"
#include "qhyexception.h"
#include "qhyimagingworker.h"
#include <chrono>
#include "c++/stlutils.h"
#include "qhycontrol.h"
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
  QList<QHYControl::ptr> controls;
  QHYImagingWorker::ptr imaging_worker;
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
  QHY_CHECK << GetQHYCCDFWVersion(d->handle, buffer) << "Getting firmware version";
  qDebug() << "firmware version: " << QString::fromLocal8Bit(reinterpret_cast<const char*>(buffer));
  double chipwidth, chipheight, pixelwidth, pixelheight;
  uint32_t width, height, bpp;
  QHY_CHECK << GetQHYCCDChipInfo(d->handle, &chipwidth, &chipheight, &width, &height, &pixelwidth, &pixelheight, &bpp) << "Getting chip information for %1" << id;
  d->chip.set_chip_size(chipwidth, chipheight).set_pixel_size(pixelwidth, pixelheight).set_resolution({static_cast<int>(width), static_cast<int>(height)});
  d->chip << Properties::Property{"bpp", bpp};
  d->chip << LiveStream;
  qDebug() << d->chip;
  d->controls = QHYControl::availableControls(d->handle);
  qDebug() << "Finished initializing QHY camera" << d->name;
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

QList<Imager::Control> QHYCCDImager::controls() const
{
    QList< Imager::Control > controls;
    transform(begin(d->controls), end(d->controls), back_inserter(controls), [](const auto &c) { return c->control(); });
  return controls;
}


void QHYCCDImager::setControl(const Imager::Control& control)
{
    auto qhyControlIt = find_if(begin(d->controls), end(d->controls), [&](const auto &c) { return c->id() == control.id; });
    if(qhyControlIt == end(d->controls))
    // TODO: error check?
        return;
    auto qhyControl = *qhyControlIt;
  wait_for(push_job_on_thread([=]{
      qhyControl->setValue(control.value.toDouble());
      qhyControl->reload();
      emit changed(qhyControl->control());
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

