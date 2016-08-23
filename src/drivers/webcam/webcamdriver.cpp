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

#include "webcamdriver.h"
#include "webcamimager.h"
#include <QDir>
#include <QDebug>
#include "Qt/strings.h"

using namespace std;

class WebcamDriver::Private
{
public:
  Private ( WebcamDriver *q );

private:
  WebcamDriver *q;
};

WebcamDriver::Private::Private ( WebcamDriver* q ) : q ( q )
{
}

WebcamDriver::WebcamDriver()
  : dptr ( this )
{
}

WebcamDriver::~WebcamDriver()
{
}


class WebcamDeviceInfo : public Driver::Camera {
public:
  WebcamDeviceInfo(int index, const QString &name) : _index {index}, _name{name} {}
  virtual Imager * imager ( const ImageHandlerPtr& imageHandler ) const { return make_shared<WebcamImager>(_name , _index, imageHandler); }
  virtual QString name() const { return _name; }
private:
  int _index;
  const QString _name;
};

Driver::Cameras WebcamDriver::cameras() const
{
  QList<CameraPtr> _cameras;
#ifdef Q_OS_LINUX
  auto entries = QDir("/sys/class/video4linux").entryInfoList();
  for(auto entry: entries) {
    QFile name_file(entry.absoluteFilePath() + "/" + "name");
    if(! name_file.exists() || ! name_file.open(QFile::ReadOnly))
      continue;
    QString name{name_file.readAll()};
    name = name.trimmed();
    int index = entry.baseName().remove("video").toInt();
    qDebug() << entry.baseName() << name  << index;
    _cameras.push_back(make_shared<WebcamDeviceInfo>(index, "%1 (OpenCV VideoCapture)"_q % name));
  }
#endif
  return _cameras;
}


