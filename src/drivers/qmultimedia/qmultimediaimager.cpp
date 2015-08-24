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
#include <QCameraInfo>
#include "qmultimediaimager.h"

class QMultimediaImager::Private {
public:
  Private(const QCameraInfo &cameraInfo, QMultimediaImager *q);
  QCameraInfo cameraInfo;
private:
  QMultimediaImager *q;
};


QMultimediaImager::Private::Private(const QCameraInfo& cameraInfo, QMultimediaImager* q) : cameraInfo{cameraInfo}, q{q}
{
  
}


QMultimediaImager::QMultimediaImager(const QCameraInfo& cameraInfo) : dptr(cameraInfo, this)
{
}

QMultimediaImager::~QMultimediaImager()
{
}

Imager::Chip QMultimediaImager::chip() const
{
  return {};
}

QString QMultimediaImager::name() const
{
  return d->cameraInfo.description();
}

Imager::Settings QMultimediaImager::settings() const
{
  return {};
}

void QMultimediaImager::setSetting(const Imager::Setting& setting)
{

}

void QMultimediaImager::startLive()
{

}

void QMultimediaImager::stopLive()
{

}


#include "qmultimediaimager.moc"
