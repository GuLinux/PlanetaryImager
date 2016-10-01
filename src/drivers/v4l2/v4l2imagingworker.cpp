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

#include "v4l2imagingworker.h"
#include <linux/videodev2.h>
#include "Qt/benchmark.h"
#include "v4l2buffer.h"
#include "v4l2utils.h"
#include <QDebug>

using namespace std;



DPTR_IMPL(V4L2ImagingWorker) {
  V4L2Device::ptr device;
  v4l2_format format;
  V4LBuffer::List buffers;
  uint32_t bufferinfo_type;
  void adjust_framerate();
  int request_buffers(int count);
};

V4L2ImagingWorker::V4L2ImagingWorker(const V4L2Device::ptr& device, const v4l2_format& format) : dptr(device, format)
{
  qDebug() << "Starting v4l2 worker with format=" << FOURCC2QS(format.fmt.pix.pixelformat) << ", res=" << "%1x%2"_q % format.fmt.pix.width % format.fmt.pix.height;
  d->adjust_framerate();
  int buffers = d->request_buffers(4);
  for(uint32_t i=0; i< buffers; i++) {
    d->buffers.push_back( make_shared<V4LBuffer>(i, d->device));
    d->buffers[i]->queue();
  }
  d->bufferinfo_type = d->buffers[0]->type();
  d->device->ioctl(VIDIOC_STREAMON, &d->bufferinfo_type, "starting streaming");
}

V4L2ImagingWorker::~V4L2ImagingWorker()
{
  d->device->ioctl(VIDIOC_STREAMOFF, &d->bufferinfo_type, "stopping live");
  d->buffers.clear();
  qDebug() << "live stopped";
  d->request_buffers(0);
  qDebug() << "requested  0 buffers";
}

void V4L2ImagingWorker::Private::adjust_framerate()
{
    v4l2_frmivalenum fps_s;
    QList<v4l2_frmivalenum> rates;
    fps_s.index = 0;
    fps_s.width = format.fmt.pix.width;
    fps_s.height = format.fmt.pix.height;
    fps_s.pixel_format = format.fmt.pix.pixelformat;
    qDebug() << "scanning for fps with pixel format " << FOURCC2QS(fps_s.pixel_format);
    for(fps_s.index = 0; device->xioctl(VIDIOC_ENUM_FRAMEINTERVALS, &fps_s) >= 0; fps_s.index++) {
      qDebug() << "found fps: " << fps_s;
      rates.push_back(fps_s);
    }
    auto ratio = [=](const v4l2_frmivalenum &a) { return static_cast<double>(a.discrete.numerator)/static_cast<double>(a.discrete.denominator); };
    sort(begin(rates), end(rates), [&](const v4l2_frmivalenum &a, const v4l2_frmivalenum &b){ return ratio(a) < ratio(b);} );
    v4l2_streamparm streamparam;
    streamparam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     device->ioctl(VIDIOC_G_PARM, &streamparam, "getting stream parameters");
    qDebug() << "current frame rate: " << streamparam.parm.capture.timeperframe;
    streamparam.parm.capture.timeperframe = rates[0].discrete;
    device->ioctl(VIDIOC_S_PARM, &streamparam, "setting stream parameters");
    qDebug() << "current frame rate: " << streamparam.parm.capture.timeperframe;
}

int V4L2ImagingWorker::Private::request_buffers(int count)
{
  v4l2_requestbuffers bufrequest;
  bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufrequest.memory = V4L2_MEMORY_MMAP;
  bufrequest.count = count;
  bufrequest.reserved[0] = 0;
  bufrequest.reserved[1] = 0;
  device->ioctl(VIDIOC_REQBUFS, &bufrequest, "requesting buffers");
  return bufrequest.count;
}


Frame::ptr V4L2ImagingWorker::shoot()
{
  Frame::ColorFormat color_format = Frame::RGB;
  QBENCH(dequeue_buffer)->every(100)->ms();
  auto buffer = d->buffers.dequeue(d->device);
  BENCH_END(dequeue_buffer);
  QBENCH(decode_image)->every(100)->ms();
  cv::Mat image{static_cast<int>(d->format.fmt.pix.height), static_cast<int>(d->format.fmt.pix.width), CV_8UC3};
  if(d->format.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG) {
      color_format = Frame::BGR;
      cv::InputArray inputArray{buffer->bytes(),  static_cast<int>(buffer->size())};
      image = cv::imdecode(inputArray, -1);
  } else if(d->format.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) {
      cv::Mat source{static_cast<int>(d->format.fmt.pix.height), static_cast<int>(d->format.fmt.pix.width), CV_8UC2, buffer->bytes() };
      cv::cvtColor(source, image, CV_YUV2RGB_YUYV);
  } else {
      qCritical() << "Unsupported image format: " << FOURCC2QS(d->format.fmt.pix.pixelformat);
      return {}; // TODO: throw exception?
  }
  BENCH_END(decode_image);
  buffer->queue(); // TODO: other types?
  return make_shared<Frame>(color_format, image);
}

