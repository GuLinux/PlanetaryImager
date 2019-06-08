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

#include "saveimages.h"
#include <QObject>
#include "Qt/qt_strings_helper.h"

using namespace std;


SaveImages::SaveImages(QObject* parent) : QObject(parent)
{
}


SaveImages::~SaveImages()
{
}

SaveImages::Error::Error(const QString &what) : runtime_error(what.toStdString()) {}

SaveImages::Error SaveImages::Error::openingFile(const QString& file, const QString& additionalInfo)
{
  auto message = QObject::tr("Unable to open video file %1 for writing") % file;
  if(!additionalInfo.isEmpty())
    message = "%1\n%2"_q % message % additionalInfo;
  return Error(message);
}
