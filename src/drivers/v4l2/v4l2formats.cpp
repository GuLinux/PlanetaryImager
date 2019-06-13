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

#include "v4l2formats.h"
#include <linux/videodev2.h>
#include <QDebug>
#include "v4l2utils.h"
#include "Qt/qt_strings_helper.h"
#include "v4l2device.h"

using namespace std;

DPTR_IMPL(V4L2Formats) {
  V4L2DevicePtr device;
  QList<V4L2FormatPtr> formats;
};

DPTR_IMPL(V4L2Format) {
  v4l2_fmtdesc fmtdesc;
  V4L2DevicePtr device;
  QList<V4L2ResolutionPtr> resolutions;
};

DPTR_IMPL(V4L2Resolution) {
  v4l2_frmsizeenum frmsizeenum;
  V4L2DevicePtr device;
  V4L2Format &format;
};

QDebug operator<<(QDebug dbg, const V4L2Resolution &resolution) {
  return dbg << "%1x%2"_q % resolution.size().width() % resolution.size().height();
}

QDebug operator<<(QDebug dbg, const V4L2Format &format) {
  dbg.nospace().noquote() << format.name() << "(" << format.description() << ")" << ": ";
  for(auto resolution: format.resolutions())
    dbg << *resolution << ", ";
  return dbg.space().quote();
}

V4L2Formats::V4L2Formats(const V4L2DevicePtr& device) : dptr(device)
{
  v4l2_fmtdesc formats;
  formats.index = 0;
  formats.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  for(formats.index = 0; 0 ==device->xioctl(VIDIOC_ENUM_FMT, &formats); formats.index++) {
    d->formats.push_back(make_shared<V4L2Format>(formats, device));
  }
  for(auto format: d->formats)
    qDebug() << "Found format: " << *format;
  auto resolution = current_resolution();
  if(resolution)
    qDebug() << "Current format: " << resolution->format().name() << ", resolution: " << *resolution;
}



namespace {
  v4l2_format v4l2_query_format(const V4L2DevicePtr &device) {
    v4l2_format current_format;
    current_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    device->ioctl(VIDIOC_G_FMT , &current_format, "querying webcam format");
    return current_format;
  }
}
v4l2_format V4L2Formats::current_v4l2_format() const
{
  return v4l2_query_format(d->device);
}

V4L2ResolutionPtr V4L2Formats::current_resolution() const
{
  // TODO: throw exception if unable to find current format/resolution?
  auto current_format = v4l2_query_format(d->device);
  auto format = find_if(d->formats.begin(), d->formats.end(), [&](const V4L2FormatPtr &f) { return current_format.fmt.pix.pixelformat == f->fourcc(); });
  if(format == d->formats.end())
    return {};
  auto resolutions = (*format)->resolutions();
  auto resolution = find_if(resolutions.begin(), resolutions.end(), [&](const V4L2ResolutionPtr &r){ 
    return 
      r->size().width() == current_format.fmt.pix.width && 
      r->size().height() == current_format.fmt.pix.height;
  });
  if(resolution == resolutions.end())
    return {};
  return *resolution;
}


V4L2Format::V4L2Format(const v4l2_fmtdesc &fmtdesc, const V4L2DevicePtr &device) : dptr(fmtdesc, device)
{
  v4l2_frmsizeenum frmsize;
  frmsize.pixel_format = fmtdesc.pixelformat;
  for(frmsize.index = 0; device->xioctl(VIDIOC_ENUM_FRAMESIZES, &frmsize, "querying resolutions") >= 0; frmsize.index++) {
    d->resolutions.push_back(make_shared<V4L2Resolution>(frmsize, device, *this));
  }
}

uint32_t V4L2Format::fourcc() const
{
  return d->fmtdesc.pixelformat;
}


QString V4L2Format::name() const
{
  return FOURCC2QS(fourcc() );
}

QString V4L2Format::description() const
{
  return QString::fromLatin1(reinterpret_cast<const char*>(d->fmtdesc.description));
}


QList<V4L2ResolutionPtr> V4L2Format::resolutions() const {
  return d->resolutions;
}


QList<V4L2FormatPtr> V4L2Formats::formats() const {
  return d->formats;
}

V4L2Resolution::V4L2Resolution(const v4l2_frmsizeenum& frmsizeenum, const V4L2DevicePtr& device, V4L2Format &format) : dptr(frmsizeenum, device, format)
{
}

V4L2Format & V4L2Resolution::format()
{
  return d->format;
}

QSize V4L2Resolution::size() const
{
  return {static_cast<int>(d->frmsizeenum.discrete.width), static_cast<int>(d->frmsizeenum.discrete.height)};
}

std::size_t V4L2Resolution::area() const
{
  return size().width() * size().height();
}

void V4L2Format::set(V4L2Resolution *resolution)
{
  auto current_format = v4l2_query_format(d->device);
  current_format.fmt.pix.pixelformat = d->fmtdesc.pixelformat;
  QSize max_resolution_size = resolution == nullptr ? max_resolution()->size() : resolution->size();
  current_format.fmt.pix.width = max_resolution_size.width();
  current_format.fmt.pix.height = max_resolution_size.height();
  d->device->ioctl(VIDIOC_S_FMT , &current_format, "setting webcam format/resolution");
}

V4L2ResolutionPtr V4L2Format::max_resolution() const
{
  return *max_element(d->resolutions.begin(), d->resolutions.end(), [](const V4L2ResolutionPtr &r1, const V4L2ResolutionPtr &r2) { return r1->area() < r2->area(); });
}

void V4L2Resolution::set()
{
  d->format.set(this);
}

uint32_t V4L2Resolution::index() const {
  return d->frmsizeenum.index;
}

V4L2Formats::~V4L2Formats()
{
}

V4L2Format::~V4L2Format()
{
}

V4L2Resolution::~V4L2Resolution()
{
}

bool V4L2Format::operator==(const V4L2Format& other) const
{
  return other.fourcc() == fourcc();
}

bool V4L2Resolution::operator==(const V4L2Resolution& other) const
{
  return other.d->format == d->format && other.size() == size();
}
