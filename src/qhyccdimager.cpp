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

#include <libusb.h>
#include "utils.h"
using namespace std;

class QHYCCDImager::Private {
public:
  Private(QHYCCDImager *q);
  qhyccd_handle *handle;
private:
  QHYCCDImager *q;
};

QHYCCDImager::Private::Private(QHYCCDImager* q) : q{q}
{

}

QHYCCDImager::QHYCCDImager(QHYDriver::Camera camera) : dpointer(this)
{
  d->handle = OpenQHYCCD(camera.id);
  if(d->handle < QHYCCD_SUCCESS) {
    throw QHYDriver::error("Initializing Camera %1"_q % camera.id, (long)(d->handle));
  }
  if(int result = InitQHYCCD(d->handle) != QHYCCD_SUCCESS) {
    throw QHYDriver::error("Initializing Camera %1"_q % camera.id, result);
  }
  qDebug() << "Camera " << camera.id << "initialized correctly";
  qDebug() << "gain: " << GetQHYCCDParam(d->handle, CONTROL_GAIN) << ", gamma: " << GetQHYCCDParam(d->handle, CONTROL_GAMMA) << ", exposure: " << GetQHYCCDParam(d->handle, CONTROL_EXPOSURE);
  char buf[1024];
  uint8_t st[1024];
  // Not implemented in QHY Library
//   GetQHYCCDCFWStatus(d->handle, buf);
//   qDebug() << "firmware status: " << QByteArray{buf};
  GetQHYCCDFWVersion(d->handle, st);
  copy(begin(st), end(st), begin(buf));
  qDebug() << "firmware version: " << QByteArray{buf};
  double chipw, chiph, pixelw, pixelh;
  int imagew, imageh, bpp;
  GetQHYCCDChipInfo(d->handle, &chipw, &chiph, &imagew, &imageh, &pixelw, &pixelh, &bpp);
  qDebug() << "chipw=" << chipw << "chiph=" << chiph << "imagew=" << imagew << "imageh=" << imageh << "pixelw=" << pixelw << "pixelh=" << pixelh << "bpp=" << bpp;
//   GetQHYCCDCameraStatus(d->handle, st); // NOT IMPLEMENTED IN QHY Library
}


QHYCCDImager::~QHYCCDImager()
{
  CloseQHYCCD(d->handle);
}
