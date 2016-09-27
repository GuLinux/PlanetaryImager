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
#include "qhydriver.h"
#include "qhyccdimager.h"
#include "qhyccd.h"
#include <map>
#include "commons/utils.h"
#include <QDebug>
#include <QString>
#include <QRegularExpression>
#include "Qt/strings.h"
#include "qhyexception.h"

using namespace std;

DPTR_IMPL(QHYDriver) {
  QHYDriver *q;
  static map<int,QString> error_codes;
  class error : public std::runtime_error {
  public:
    error(const QString &label, int code) : runtime_error(("Error on %1: %2 (%3)"_q % label % code % error_codes[code]).toStdString()) {}
  };
};


class QHYCamera : public Driver::Camera {
public:
  QHYCamera(int index) : index{index} {}
  virtual Imager *imager(const ImageHandler::ptr& imageHandler) const { return new QHYCCDImager(name(), id, imageHandler); }
  virtual QString name() const {   return QString(id).remove(QRegularExpression{"-[\\da-f]+$"}); }
  char id[255];
  int index;
};

QHYDriver::QHYDriver() : dptr(this)
{
  QHY_CHECK << InitQHYCCDResource() << "initializing QHY Driver";
  qDebug() << "Initialized QHY Driver";
}

QHYDriver::~QHYDriver()
{
  QHY_CHECK << ReleaseQHYCCDResource() << "releasing QHY Driver";
  qDebug() << "Released QHY Driver";
}


Driver::Cameras QHYDriver::cameras() const
{
  Cameras cameras;
  int found_cameras = ScanQHYCCD();
  if(found_cameras == 0 /* was QHYCCD_ERROR_NO_DEVICE */) {
    return cameras;
  }
  QHY_CHECK << found_cameras << "Getting QHY cameras list";
  for(int i=0; i<found_cameras; i++) {
    auto camera = make_shared<QHYCamera>(i);
    QHY_CHECK << GetQHYCCDId(i, &camera->id[0]) << "Getting QHY camera ID";
    qDebug() << "Found device at index " << i << " with id=" << camera->id << ")";
    cameras.push_back(camera);
  }
  return cameras;
}

