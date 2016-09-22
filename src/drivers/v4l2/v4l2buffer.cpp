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

#include "v4l2buffer.h"
#include "v4l2exception.h"
#include <sys/mman.h>

using namespace std;

DPTR_IMPL(V4LBuffer) {
  V4L2Device::ptr v4ldevice;
  v4l2_buffer bufferinfo;
  char *memory;
};


V4LBuffer::V4LBuffer(int index, const V4L2Device::ptr &v4ldevice) : dptr(v4ldevice)
{
  memset(&d->bufferinfo, 0, sizeof(v4l2_buffer));
  d->bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  d->bufferinfo.memory = V4L2_MEMORY_MMAP;
  d->bufferinfo.index = index;
  
  v4ldevice->ioctl(VIDIOC_QUERYBUF, &d->bufferinfo, "allocating buffers");

  d->memory = (char*) mmap(NULL, d->bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, v4ldevice->descriptor(), d->bufferinfo.m.offset);
  V4L2_CHECK << ( d->memory==MAP_FAILED ? -1 : 0 )<< "mapping memory";
  memset(d->memory, 0, d->bufferinfo.length);
}

void V4LBuffer::queue()
{
  d->v4ldevice->ioctl(VIDIOC_QBUF, &d->bufferinfo, "queuing buffer");
}

std::shared_ptr< V4LBuffer > V4LBuffer::List::dequeue(const shared_ptr<V4L2Device> &device) const
{
    v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    device->ioctl(VIDIOC_DQBUF, &bufferinfo, "dequeuing buffer");
    auto buffer = this->at(bufferinfo.index);
    buffer->d->bufferinfo = bufferinfo;
    return buffer;
}


V4LBuffer::~V4LBuffer()
{
  try {
  V4L2_CHECK << munmap(d->memory, d->bufferinfo.length) << "unmapping memory";
  }
  catch(const V4L2Exception &e) {
    qWarning() << "error unmapping memory: " << strerror(errno);
    return;
  }
  qDebug() << "memory map deleted";
}

char *V4LBuffer::bytes() const
{
  return d->memory;
}

uint32_t V4LBuffer::size() const
{
  return d->bufferinfo.bytesused;
}

uint32_t V4LBuffer::type() const
{
  return d->bufferinfo.type;
}
