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

#include "webcamimager.h"
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

class WebcamImager::Private
{
public:
  Private ( const QString &name, int index, const ImageHandlerPtr &handler, WebcamImager *q );
  const QString name;
  int index;
  ImageHandlerPtr handler;
  bool live = false;
private:
  WebcamImager *q;
};

WebcamImager::Private::Private ( const QString& name, int index, const ImageHandlerPtr& handler, WebcamImager* q ) 
  : name{name}, index{index}, handler{handler}, q{q}
{
}

WebcamImager::WebcamImager(const QString &name, int index, const ImageHandlerPtr &handler)
  : dptr ( name, index, handler, this )
{
}



WebcamImager::~WebcamImager()
{
}

Imager::Chip WebcamImager::chip() const
{
  return {};
}

QString WebcamImager::name() const
{
  return d->name;
}

Imager::Settings WebcamImager::settings() const
{
  return {};
}

void WebcamImager::setSetting ( const Imager::Setting& setting )
{

}

void WebcamImager::startLive()
{
  d->live = true;
  QtConcurrent::run([=]{
    cv::VideoCapture capture(d->index);
    if(!capture.isOpened()) {
      qDebug() << "error opening device";
      return;
    }
    while(d->live) {
      cv::Mat frame;
      capture >> frame;
      d->handler->handle(frame);
    }
  });
}

void WebcamImager::stopLive()
{
  d->live = false;
}
