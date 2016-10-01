/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
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

#include "qhyimagingworker.h"
#include "qhyexception.h"
#include "qhyccd.h"
#include <QDebug>
using namespace std;

DPTR_IMPL(QHYImagingWorker) {
  qhyccd_handle *handle;
  QHYImagingWorker *q;
  uint32_t w, h, bpp, channels;
  
  Frame::ColorFormat color_format;
  vector<uint8_t> buffer;
  static bool is_zero(uint8_t b);
};

QHYImagingWorker::QHYImagingWorker(qhyccd_handle *handle) : dptr(handle, this)
{
  static map<int, Frame::ColorFormat> color_formats {
    {BAYER_GB, Frame::Bayer_GBRG},
    {BAYER_GR, Frame::Bayer_GRBG},
    {BAYER_BG, Frame::Bayer_BGGR},
    {BAYER_RG, Frame::Bayer_RGGB},
  };
  int colorret = IsQHYCCDControlAvailable(d->handle,CAM_COLOR);
  if(color_formats.count(colorret)) {
    d->color_format = color_formats[colorret];
    SetQHYCCDDebayerOnOff(d->handle, false);
  } else {
    d->color_format = Frame::Mono;
  }
  d->buffer.resize(GetQHYCCDMemLength(d->handle));
  
  QHY_CHECK << SetQHYCCDStreamMode(d->handle,1) << "Unable to set live mode stream";
  QHY_CHECK << BeginQHYCCDLive(d->handle) << "Unable to start live mode";
}

QHYImagingWorker::~QHYImagingWorker()
{
  QHY_CHECK << StopQHYCCDLive(d->handle) << "Unable to stop live mode stream";
}


Frame::ptr QHYImagingWorker::shoot()
{
  QHY_CHECK << GetQHYCCDLiveFrame(d->handle,&d->w,&d->h,&d->bpp,&d->channels,d->buffer.data()) << "Capturing live frame";
  
  if(all_of(d->buffer.begin(), d->buffer.end(), &Private::is_zero )) {
    qWarning() << "Frame is all empty, skipping";
    return {};
  } else {
    int type = d->bpp==8 ? CV_8UC1 : CV_16UC1;
    if(d->channels == 3)
      type = d->bpp==8 ? CV_8UC3 : CV_16UC3;
    cv::Mat image({static_cast<int>(d->w), static_cast<int>(d->h)}, type, d->buffer.data());
    cv::Mat copy;
    image.copyTo(copy); // TODO: is copy necessary?
    return make_shared<Frame>(d->color_format, copy); //TODO port to new constructor
     // TODO: Properly handle with debayer setting, I guess... find a tester!
  }
}

bool QHYImagingWorker::Private::is_zero(uint8_t b)
{
  return b == 0;
}
