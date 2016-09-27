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

#include "v4l2driver.h"
#include "v4l2imager.h"
#include <QDir>
#include <QDebug>
#include "Qt/strings.h"

using namespace std;

DPTR_IMPL(V4L2Driver) {
  V4L2Driver *q;
};

V4L2Driver::V4L2Driver()
  : dptr ( this )
{
}

V4L2Driver::~V4L2Driver()
{
}


class V4L2DeviceInfo : public Driver::Camera {
public:
  V4L2DeviceInfo(int index, const QString &name) : _index {index}, _name{name} {}
  virtual Imager * imager ( const ImageHandler::ptr& imageHandler ) const { return new V4L2Imager(_name , _index, imageHandler); }
  virtual QString name() const { return _name; }
private:
  int _index;
  const QString _name;
};

Driver::Cameras V4L2Driver::cameras() const
{
  Cameras _cameras;
  auto entries = QDir("/sys/class/video4linux").entryInfoList();
  for(auto entry: entries) {
    QFile name_file(entry.absoluteFilePath() + "/" + "name");
    if(! name_file.exists() || ! name_file.open(QFile::ReadOnly))
      continue;
    QString name{name_file.readAll()};
    name = name.trimmed();
    int index = entry.baseName().remove("video").toInt();
    qDebug() << entry.baseName() << name  << index;
    _cameras.push_back(make_shared<V4L2DeviceInfo>(index, "%1 (v4l2)"_q % name));
  }
  return _cameras;
}


