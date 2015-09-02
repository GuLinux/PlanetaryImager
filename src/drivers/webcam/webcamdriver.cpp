/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <marco.gulino@bhuman.it>
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
#include <QDir>
#include <QDebug>
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

Driver::Cameras WebcamDriver::cameras() const
{
#ifdef Q_OS_LINUX
  auto entries = QDir("/sys/class/video4linux").entryInfoList();
  for(auto entry: entries) {
    QFile index_file(entry.absoluteFilePath() + "/" + "name");
    QFile name_file(entry.absoluteFilePath() + "/" + "name");
    if(!index_file.exists() || ! name_file.exists() || ! name_file.open(QFile::ReadOnly) || ! index_file.open(QFile::ReadOnly))
      continue;
    QString name{name_file.readAll()};
    name = name.trimmed();
    int index = QString{index_file.readAll()}.toInt();
    qDebug() << entry.baseName() << name << index;
  }
#endif
  return {};
}
