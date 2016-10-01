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

#include "v4l2device.h"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "Qt/strings.h"
#include <QDebug>
#include "c++/stringbuilder.h"
#include "v4l2exception.h"
#include <linux/videodev2.h>

using namespace std;

DPTR_IMPL(V4L2Device) {
  const QString path;
  QMap<int, QString> ioctl_names;
  int fd;
};

#define DESCRIBE_IOCTL(name) d->ioctl_names[name] = #name;

V4L2Device::V4L2Device(const QString& path) : dptr(path)
{
    d->fd = ::open(path.toLatin1(), O_RDWR, O_NONBLOCK, 0);
    V4L2_CHECK << d->fd << "opening device '%1'"_q % d->path;
    DESCRIBE_IOCTL(VIDIOC_DQBUF)
    DESCRIBE_IOCTL(VIDIOC_ENUM_FMT)
    DESCRIBE_IOCTL(VIDIOC_ENUM_FRAMEINTERVALS)
    DESCRIBE_IOCTL(VIDIOC_ENUM_FRAMESIZES)
    DESCRIBE_IOCTL(VIDIOC_G_CTRL)
    DESCRIBE_IOCTL(VIDIOC_G_FMT)
    DESCRIBE_IOCTL(VIDIOC_G_PARM)
    DESCRIBE_IOCTL(VIDIOC_QBUF)
    DESCRIBE_IOCTL(VIDIOC_QUERYBUF)
    DESCRIBE_IOCTL(VIDIOC_QUERYCAP)
    DESCRIBE_IOCTL(VIDIOC_QUERYCTRL)
    DESCRIBE_IOCTL(VIDIOC_QUERYMENU)
    DESCRIBE_IOCTL(VIDIOC_REQBUFS)
    DESCRIBE_IOCTL(VIDIOC_S_CTRL)
    DESCRIBE_IOCTL(VIDIOC_S_FMT)
    DESCRIBE_IOCTL(VIDIOC_S_PARM)
    DESCRIBE_IOCTL(VIDIOC_STREAMOFF)
    DESCRIBE_IOCTL(VIDIOC_STREAMON)
}

V4L2Device::~V4L2Device()
{
  if(-1 != d->fd)
    ::close(d->fd);
}

int V4L2Device::descriptor() const
{
  return d->fd;
}

V4L2Device::operator bool() const
{
   return d->fd != -1;
}

QString V4L2Device::path() const
{
  return d->path;
}


void V4L2Device::__ioctl(uint64_t ctl, void* data, const QString& errorLabel) const
{
    int r;
    do {
        r = ::ioctl(d->fd, ctl, data);
    } while (-1 == r && EINTR == errno);
    V4L2_CHECK << r << (errorLabel.isEmpty() ? "ioctl %1"_q % d->ioctl_names.value(ctl, "%1"_q % ctl) : errorLabel);
    //qDebug() << "IOCTL RUN: %1 with result %2"_q % d->ioctl_names.value(ctl, "%1"_q % ctl) % r;
}

int V4L2Device::__xioctl(uint64_t ctl, void* data, const QString& errorLabel) const
{
  try {
    this->ioctl(ctl, data, errorLabel);
    return 0;
  } catch(const V4L2Exception &e) {
    qWarning() << e.what();
    return -1;
  }
}

