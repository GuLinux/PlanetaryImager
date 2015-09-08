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
#include <linux/videodev2.h>
#include "Qt/strings.h"
#include "fps_counter.h"

#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

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
  void read_v4l2_parameters();
  struct Resolution {
    int width, height;
    bool operator<(const Resolution &other) const { return width*height < other.width*other.height; };
    bool operator==(const Resolution &other) const { return width == other.width && height == other.height; }
    operator bool() const { return width > 0 && height > 0; }
    operator QString() const { return "%1x%2"_q % width % height; }
    friend QDebug operator<<(QDebug d, const WebcamImager::Private::Resolution &r) {
      d.nospace() << "{" << r.operator QString() << "}";
      return d.space();
    }
  };
  QList<Resolution> resolutions;
  int fd;
private:
  WebcamImager *q;
};



struct v4l2_queryctrl queryctrl;
struct v4l2_querymenu querymenu;
struct v4l2_control control;
// http://www.linuxtv.org/downloads/legacy/video4linux/API/V4L2_API/spec-single/v4l2.html#control

static void enumerate_menu (int fd)
{
        qDebug()<< "  Menu items:";

        memset (&querymenu, 0, sizeof (querymenu));
        querymenu.id = queryctrl.id;

        for (querymenu.index = queryctrl.minimum;
             querymenu.index <= queryctrl.maximum;
              querymenu.index++) {
                if (0 == ioctl (fd, VIDIOC_QUERYMENU, &querymenu)) {
                        qDebug() << "    " << (char*) querymenu.name << ":" << querymenu.value;
                } else {
                        qDebug() << "VIDIOC_QUERYMENU error:" << strerror (errno);
                        exit (EXIT_FAILURE);
                }
        }
}


WebcamImager::Private::Private ( const QString& name, int index, const ImageHandlerPtr& handler, WebcamImager* q ) 
  : name{name}, index{index}, handler{handler}, q{q}
{

}

WebcamImager::WebcamImager(const QString &name, int index, const ImageHandlerPtr &handler)
  : dptr ( name, index, handler, this )
{
  d->read_v4l2_parameters();
  d->capture = make_shared<cv::VideoCapture>(d->index);
  if(!d->capture->isOpened()) {
    qDebug() << "error opening device";
  }
  d->capture->set(CV_CAP_PROP_FRAME_WIDTH, d->resolutions.last().width);
  d->capture->set(CV_CAP_PROP_FRAME_HEIGHT, d->resolutions.last().height);
}


QDebug operator <<(QDebug dbg, const v4l2_queryctrl &q) {
  static QMap<int, QString> types {
        { V4L2_CTRL_TYPE_INTEGER, "V4L2_CTRL_TYPE_INTEGER" },
        { V4L2_CTRL_TYPE_BOOLEAN, "V4L2_CTRL_TYPE_BOOLEAN" },
        { V4L2_CTRL_TYPE_MENU, "V4L2_CTRL_TYPE_MENU" },
        { V4L2_CTRL_TYPE_BUTTON, "V4L2_CTRL_TYPE_BUTTON" },
        { V4L2_CTRL_TYPE_INTEGER64, "V4L2_CTRL_TYPE_INTEGER64" },
        { V4L2_CTRL_TYPE_CTRL_CLASS, "V4L2_CTRL_TYPE_CTRL_CLASS" },
        { V4L2_CTRL_TYPE_STRING, "V4L2_CTRL_TYPE_STRING" },
        { V4L2_CTRL_TYPE_BITMASK, "V4L2_CTRL_TYPE_BITMASK" },
        { V4L2_CTRL_TYPE_INTEGER_MENU, "V4L2_CTRL_TYPE_INTEGER_MENU" },
        { V4L2_CTRL_COMPOUND_TYPES, "V4L2_CTRL_COMPOUND_TYPES" },
        { V4L2_CTRL_TYPE_U8, "V4L2_CTRL_TYPE_U8" },
        { V4L2_CTRL_TYPE_U16, "V4L2_CTRL_TYPE_U16" },
        { V4L2_CTRL_TYPE_U32, "V4L2_CTRL_TYPE_U32" },
  };
  dbg.nospace() << "v4l2_queryctrl{";
  dbg << "type=" << types[q.type] << ", name=" << (char*)(q.name)<< ", default_value=" << q.default_value 
    << ", min=" << q.minimum << ", max=" << q.maximum << ", step=" << q.step;
  dbg << "}";
  return dbg.space();
}

void WebcamImager::Private::read_v4l2_parameters()
{
  auto dev_name = "/dev/video%1"_q % index;
  fd = ::open(dev_name.toLatin1(), O_RDWR, O_NONBLOCK, 0);
  if (-1 == fd) {
    qWarning() << "Cannot open '%1': %2, %3"_q % dev_name % errno % strerror(errno);
    return;
  }
  
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  struct v4l2_fmtdesc fmt;
  struct v4l2_frmsizeenum frmsize;
  struct v4l2_frmivalenum frmival;

  fmt.index = 0;
  fmt.type = type;
  while (::ioctl(fd, VIDIOC_ENUM_FMT, &fmt) >= 0) {
      frmsize.pixel_format = fmt.pixelformat;
      frmsize.index = 0;
      while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {
        Resolution resolution;
        if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
          resolution = {frmsize.discrete.width , frmsize.discrete.height};
        } else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
          resolution = {frmsize.stepwise.max_width , frmsize.stepwise.max_height};
        }
          if(resolution && ! resolutions.contains(resolution))
            resolutions.push_back(resolution);
          frmsize.index++;
        }
        fmt.index++;
  }
  qSort(resolutions);
  qDebug() << "available resolutions: " << resolutions;
  
  
  
  memset (&queryctrl, 0, sizeof (queryctrl));

for (queryctrl.id = V4L2_CID_BASE;
     queryctrl.id < V4L2_CID_LASTP1;
     queryctrl.id++) {
        if (0 == ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl)) {
                if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                        continue;
                control.id = queryctrl.id;
        if (-1 == ioctl (fd, VIDIOC_G_CTRL, &control)) {
                qDebug() << "error on VIDIOC_G_CTRL" << strerror (errno);
        }
                qDebug() << "Control "<<  queryctrl << ", value: " << control.value;

                if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
                        enumerate_menu (fd);
        } else {
                if (errno == EINVAL)
                        continue;

                qDebug() << "Error on VIDIOC_QUERYCTRL" <<  strerror (errno);
                exit (EXIT_FAILURE);
        }
}
qDebug() << "PRIVATE BASE:";
for (queryctrl.id = V4L2_CID_PRIVATE_BASE;;
     queryctrl.id++) {
        if (0 == ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl)) {
                if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                        continue;

                qDebug() << "Control "<< queryctrl;

                if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
                        enumerate_menu (fd);
        } else {
                if (errno == EINVAL)
                        break;

                qDebug() << "error on VIDIOC_QUERYCTRL" << strerror (errno);
                exit (EXIT_FAILURE);
        }
}
  
  
}




WebcamImager::~WebcamImager()
{
  stopLive();
  ::close(d->fd);
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
    fps_counter fps([=](double rate){ emit this->fps(rate);}, fps_counter::Mode::Elapsed);

    while(d->live) {
      cv::Mat frame;
      *d->capture >> frame;
      d->handler->handle(frame);
      ++fps;
    }
  });
}

void WebcamImager::stopLive()
{
  d->live = false;
}
