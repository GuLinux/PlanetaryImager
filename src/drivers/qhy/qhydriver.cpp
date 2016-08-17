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
#include "utils.h"
#include <QDebug>
#include <QString>
#include <QRegularExpression>
#include "Qt/strings.h"

using namespace std;

class QHYDriver::Private {
public:
  Private(QHYDriver *q);
  static map<int,QString> error_codes;
  static map<char,QString> device_codes;
  class error : public std::runtime_error {
  public:
    error(const QString &label, int code) : runtime_error(("Error on %1: %2 (%3)"_q % label % code % error_codes[code]).toStdString()) {}
  };
private:
  QHYDriver *q;
};

#include "qhy_messages.cpp"

class QHYCamera : public Driver::Camera {
public:
  QHYCamera(int index) : index{index} {}
  virtual ImagerPtr imager(const ImageHandlerPtr& imageHandler) const { return make_shared<QHYCCDImager>(name(), id, imageHandler); }
  virtual QString name() const {   return QString(id).remove(QRegularExpression{"-[\\da-f]+$"}); }
  char id[255];
  int index;
};

QHYDriver::Private::Private(QHYDriver* q) : q(q)
{

}

QHYDriver::error::error(const QString& label, int code):  runtime_error(("Error on %1: %2 (%3)"_q % label % code % Private::error_codes[code]).toStdString())
{
}


QString QHYDriver::error_name(int code)
{
  return Private::error_codes[code];
}

QHYDriver::QHYDriver() : dptr(this)
{
  if(int result = InitQHYCCDResource() != QHYCCD_SUCCESS)
    throw error("initializing QHY Driver", result);
  qDebug() << "Initialized QHY Driver";
}

QHYDriver::~QHYDriver()
{
  if(int result = ReleaseQHYCCDResource() != QHYCCD_SUCCESS)
    throw error("releasing QHY Driver", result);
  qDebug() << "Released QHY Driver";
}


Driver::Cameras QHYDriver::cameras() const
{
  Cameras cameras;
  int scan_result= ScanQHYCCD();
  if(scan_result == 0 /* was QHYCCD_ERROR_NO_DEVICE */) {
    return cameras;
  }
  if(scan_result < QHYCCD_SUCCESS) {
    throw error{"getting QHY cameras list", scan_result};
  }
  for(int i=0; i<scan_result; i++) {
    auto camera = make_shared<QHYCamera>(i);
    if(int result = GetQHYCCDId(i, &camera->id[0]) != QHYCCD_SUCCESS)
      throw error{"getting QHY Camera ID", result};
    qDebug() << "Found device at index " << i << " with id=" << camera->id << ")";
    cameras.push_back(camera);
  }
  return cameras;
}

