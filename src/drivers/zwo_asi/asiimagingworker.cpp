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

#include "asiimagingworker.h"
#include "zwoexception.h"
using namespace std;

DPTR_IMPL(ASIImagingWorker) {
  ASI_IMG_TYPE format;
  ASI_CAMERA_INFO info;
  int bin;
  QRect roi;

  std::vector<uint8_t> buffer;
  size_t calcBufferSize();
  int getCVImageType();
};

ASIImagingWorker::ASIImagingWorker(const QRect& requestedROI, int bin, const ASI_CAMERA_INFO& info, ASI_IMG_TYPE format)
    : dptr(format, info, bin, QRect{requestedROI.x(), requestedROI.y(), (requestedROI.width() / 4) * 4, (requestedROI.height()/4) * 4 })
{
    qDebug() << "Starting imaging: imageFormat=" << d->format << ", d->roi: " << d->roi << ", bin: " << d->bin;
    ASI_CHECK << ASISetROIFormat(d->info.CameraID, d->roi.width(), d->roi.height(), d->bin, d->format) << "Set d->format";
    ASI_CHECK << ASISetStartPos(d->info.CameraID, d->roi.x(), d->roi.y()) << "Set ROI position";
    ASI_CHECK << ASIStartVideoCapture(d->info.CameraID) << "Start video capture";
    d->buffer.resize(d->calcBufferSize());
    qDebug() << "Imaging started: imageFormat=" << d->format << ", roi: " << d->roi << ", bin: " << d->bin;
}

ASIImagingWorker::~ASIImagingWorker()
{
}

void ASIImagingWorker::start()
{
}


bool ASIImagingWorker::shoot(const ImageHandlerPtr& imageHandler)
{
  try {
    ASI_CHECK << ASIGetVideoData(d->info.CameraID, d->buffer.data(), d->buffer.size(), 100000) << "Capture frame";
        cv::Mat image( {d->roi.width(), d->roi.height()}, d->getCVImageType(), d->buffer.data());
        cv::Mat copy;
        image.copyTo(copy);
        imageHandler->handle(copy);
        return true;
  }
   catch(ZWOException &e) {
      qDebug() << QString::fromStdString(e.what());
      return false;
  }
}

void ASIImagingWorker::stop()
{
    ASI_CHECK << ASIStopVideoCapture(d->info.CameraID) << "Stop capture";
    qDebug() << "Imaging stopped.";
}

size_t ASIImagingWorker::Private::calcBufferSize()
{
    auto base_size = roi.width() * roi.height();
    switch(format) {
    case ASI_IMG_RAW8:
        return base_size;
    case ASI_IMG_RAW16:
        return base_size * 2;
    case ASI_IMG_RGB24:
        return base_size * 3;
    default:
        throw runtime_error("Format not supported");
    }
}

int ASIImagingWorker::Private::getCVImageType()
{
    switch(format) {
    case ASI_IMG_RAW8:
        return CV_8UC1;
    case ASI_IMG_RAW16:
        return CV_16UC1;
    case ASI_IMG_RGB24:
        return CV_8UC3;
    default:
        throw runtime_error("Format not supported");

    }
}

QRect ASIImagingWorker::roi() const
{
  return d->roi;
}

ASI_IMG_TYPE ASIImagingWorker::format() const
{
  return d->format;
}

int ASIImagingWorker::bin() const
{
  return d->bin;
}


