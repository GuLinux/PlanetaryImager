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

#include "v4ldevice.h"

#include <linux/videodev2.h>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "Qt/strings.h"
#include <QDebug>
#include "c++/stringbuilder.h"

using namespace std;

V4L2Device::V4L2Device(const QString& path) : _path{path}
{
    fd = ::open(path.toLatin1(), O_RDWR, O_NONBLOCK, 0);
    if (-1 == fd) {
      throw V4L2Device::exception("opening device '%1'"_q % path);
    }
}

V4L2Device::~V4L2Device()
{
  if(-1 != fd)
    ::close(fd);
}

void V4L2Device::__ioctl(uint64_t ctl, void* data, const QString& errorLabel) const
{
    int r;
    do {
        r = ::ioctl(fd, ctl, data);
    } while (-1 == r && EINTR == errno);
    if(r == -1)
      throw V4L2Device::exception(errorLabel.isEmpty() ? "ioctl %1"_q % ctl : errorLabel);
}

int V4L2Device::__xioctl(uint64_t ctl, void* data, const QString& errorLabel) const
{
  try {
    this->ioctl(ctl, data, errorLabel);
    return 0;
  } catch(V4L2Device::exception &e) {
    qWarning() << e.what();
    return -1;
  }
}



const char* V4L2Device::exception::what() const noexcept
{
  string _what = GuLinux::stringbuilder() << "V4LDevice::exception at " << label.toStdString() << ": " << strerror(_error_code);
  return _what.c_str();
}


