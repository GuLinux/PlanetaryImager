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

using namespace std;

class WebcamImager::Private
{
public:
  Private ( const QString &name, int index, const ImageHandlerPtr &handler, WebcamImager *q );
  const QString name;
  int index;
  ImageHandlerPtr handler;
  bool live = false;
  shared_ptr<cv::VideoCapture> capture;
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
  d->capture = make_shared<cv::VideoCapture>(d->index);
  if(!d->capture->isOpened()) {
    qDebug() << "error opening device";
  }
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
  static QList<Setting> defaults {
    {CV_CAP_PROP_FRAME_WIDTH, "Width", 0, std::numeric_limits<int>().max(), 0, 0},
    {CV_CAP_PROP_FRAME_HEIGHT, "Height", 0, std::numeric_limits<int>().max(), 0, 0},
    {CV_CAP_PROP_FORMAT, "Format", 0, std::numeric_limits<int>().max(), 0, 0},
    {CV_CAP_PROP_MODE, "Mode", 0, std::numeric_limits<int>().max(), 0, 0},
    {CV_CAP_PROP_BRIGHTNESS, "Brightness", 0, std::numeric_limits<int>().max(), 0, 0},
    {CV_CAP_PROP_CONTRAST, "Contrast", 0, std::numeric_limits<int>().max(), 0, 0},
    {CV_CAP_PROP_SATURATION, "Saturation", 0, std::numeric_limits<int>().max(), 0, 0},
    {CV_CAP_PROP_HUE, "Hue", 0, std::numeric_limits<int>().max(), 0, 0},
    {CV_CAP_PROP_GAIN, "Gain", 0, std::numeric_limits<int>().max(), 0, 0},
    {CV_CAP_PROP_EXPOSURE, "Exposure", 0, std::numeric_limits<int>().max(), 0, 0},
    {CV_CAP_PROP_CONVERT_RGB, "Convert RGB", 0, std::numeric_limits<int>().max(), 0, 0},
  };
  Imager::Settings _settings;
  for(auto setting: defaults) {
    setting.value = d->capture->get(setting.id);
    _settings.push_back(setting);
  }
  return _settings;
}

void WebcamImager::setSetting ( const Imager::Setting& setting )
{

}

void WebcamImager::startLive()
{
  d->live = true;
  QtConcurrent::run([=]{

    while(d->live) {
      cv::Mat frame;
      *d->capture >> frame;
      d->handler->handle(frame);
    }
  });
}

void WebcamImager::stopLive()
{
  d->live = false;
}
