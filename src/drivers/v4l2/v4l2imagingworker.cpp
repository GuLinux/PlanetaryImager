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
#include <QHash>
#include <functional>
#include "v4l2exception.h"
#include "c++/stringbuilder.h"
using namespace std;
using namespace std::placeholders;



DPTR_IMPL(V4L2ImagingWorker) {
  V4L2Device::ptr device;
  v4l2_format format;
  V4LBuffer::List buffers;
  uint32_t bufferinfo_type;
  function<Frame::ptr(const V4LBuffer::ptr)> get_frame;
  Frame::ptr import_frame(const V4LBuffer::ptr &buffer);
  Frame::ptr create_frame(const V4LBuffer::ptr &buffer, int cv_type, Frame::ColorFormat color_format);
  Frame::ptr convert_frame(const V4LBuffer::ptr &buffer, int cv_type, int cv_conversion_format, Frame::ColorFormat color_format);
};

V4L2ImagingWorker::V4L2ImagingWorker(const V4L2Device::ptr& device, const v4l2_format& format) : dptr(device, format)
{
  QHash<uint32_t, function<Frame::ptr(const V4LBuffer::ptr)>> formats = {
    // Mono Formats
    {V4L2_PIX_FMT_GREY, bind(&Private::create_frame, d.get(), _1, CV_8UC1, Frame::Mono)},
    {V4L2_PIX_FMT_Y16, bind(&Private::create_frame, d.get(), _1, CV_16UC1, Frame::Mono)},
    // RGB/BGR
    {V4L2_PIX_FMT_BGR24, bind(&Private::create_frame, d.get(), _1, CV_8UC3, Frame::BGR)},
    {V4L2_PIX_FMT_RGB24, bind(&Private::create_frame, d.get(), _1, CV_8UC3, Frame::RGB)},
    // Bayer 8bit
    {V4L2_PIX_FMT_SBGGR8, bind(&Private::create_frame, d.get(), _1, CV_8UC1, Frame::Bayer_BGGR)},
    {V4L2_PIX_FMT_SGBRG8, bind(&Private::create_frame, d.get(), _1, CV_8UC1, Frame::Bayer_GBRG)},
    {V4L2_PIX_FMT_SGRBG8, bind(&Private::create_frame, d.get(), _1, CV_8UC1, Frame::Bayer_GRBG)},
    {V4L2_PIX_FMT_SRGGB8, bind(&Private::create_frame, d.get(), _1, CV_8UC1, Frame::Bayer_RGGB)},
    // Bayer 16bit
    {V4L2_PIX_FMT_SBGGR16, bind(&Private::create_frame, d.get(), _1, CV_16UC1, Frame::Bayer_BGGR)},
    // Compressed formats
    {V4L2_PIX_FMT_MJPEG, bind(&Private::import_frame, d.get(), _1)},
    // YUV Colorspace
    {V4L2_PIX_FMT_YUYV, bind(&Private::convert_frame, d.get(), _1, CV_8UC2, CV_YUV2RGB_YUYV, Frame::RGB)},
  };
  auto pixelformat = format.fmt.pix.pixelformat;
  if(!formats.contains(pixelformat))
    throw V4L2Exception(V4L2Exception::unimplemented_error, GuLinux::stringbuilder() << "Requested format " << FOURCC2QS(pixelformat).toStdString() << " (" << pixelformat << ") not yet supported. Please report this error message for help");
  d->get_frame = formats[pixelformat];
}

V4L2ImagingWorker::~V4L2ImagingWorker()
{
}


void V4L2ImagingWorker::start()
{
  qDebug() << "format: " << FOURCC2QS(d->format.fmt.pix.pixelformat) << ", " << d->format.fmt.pix.width << "x" << d->format.fmt.pix.height;
  
  v4l2_requestbuffers bufrequest;
  bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufrequest.memory = V4L2_MEMORY_MMAP;
  bufrequest.count = 4;
  d->device->ioctl(VIDIOC_REQBUFS, &bufrequest, "requesting buffers");
  
  for(uint32_t i=0; i<bufrequest.count; i++) {
    d->buffers.push_back( make_shared<V4LBuffer>(i, d->device));
    d->buffers[i]->queue();
  }
                                  
  d->bufferinfo_type = d->buffers[0]->type();
  d->device->ioctl(VIDIOC_STREAMON, &d->bufferinfo_type, "starting streaming");
}

Frame::ptr V4L2ImagingWorker::shoot()
{
  QBENCH(dequeue_buffer)->every(100)->ms();
  auto buffer = d->buffers.dequeue(d->device);
  BENCH_END(dequeue_buffer);
  QBENCH(decode_image)->every(100)->ms();

  auto frame = d->get_frame(buffer);
  BENCH_END(decode_image);
  buffer->queue(); // TODO: other types?
  return frame;
}

Frame::ptr V4L2ImagingWorker::Private::import_frame(const V4LBuffer::ptr& buffer)
{
  cv::InputArray inputArray{buffer->bytes(),  static_cast<int>(buffer->size())};
  cv::Mat image = cv::imdecode(inputArray, -1);
  return make_shared<Frame>(Frame::BGR, image);
}


Frame::ptr V4L2ImagingWorker::Private::convert_frame(const V4LBuffer::ptr& buffer, int cv_type, int cv_conversion_format, Frame::ColorFormat color_format)
{
  cv::Mat image{static_cast<int>(format.fmt.pix.height), static_cast<int>(format.fmt.pix.width), cv_type, buffer->bytes()};
  cv::cvtColor(image, image, cv_conversion_format);
  return make_shared<Frame>(color_format, image);
}

Frame::ptr V4L2ImagingWorker::Private::create_frame(const V4LBuffer::ptr& buffer, int cv_type, Frame::ColorFormat color_format)
{
    cv::Mat image = cv::Mat{static_cast<int>(format.fmt.pix.height), static_cast<int>(format.fmt.pix.width), cv_type, buffer->bytes()};
    // copy(buffer->bytes(), buffer->bytes() + buffer->size(), image.begin<uint8_t>());
    // move(buffer->bytes(), buffer->bytes() + buffer->size(), image.begin<uint8_t>());
    return make_shared<Frame>(color_format, image);
}


void V4L2ImagingWorker::stop()
{
  d->buffers.clear();
  d->device->ioctl(VIDIOC_STREAMOFF, &d->bufferinfo_type, "stopping live");
  qDebug() << "live stopped";
}
