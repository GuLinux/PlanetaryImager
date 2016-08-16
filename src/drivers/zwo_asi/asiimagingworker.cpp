/*
 * Copyright (C) 2016 Marco Gulino (marco AT gulinux.net)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include "asiimagingworker.h"
#include "zwoexception.h"
using namespace std;



ASIImagingWorker::ASIImagingWorker(const QRect& requestedROI, int bin, const ASI_CAMERA_INFO& info, ASI_IMG_TYPE format)
    : format {format}, info {info}, bin {bin}, roi {requestedROI.x(), requestedROI.y(), (requestedROI.width() / 4) * 4, (requestedROI.height()/4) * 4 }
{
    qDebug() << "Starting imaging: imageFormat=" << format << ", roi: " << roi << ", bin: " << bin;
    ASI_CHECK << ASISetROIFormat(info.CameraID, roi.width(), roi.height(), bin, format) << "Set format";
    ASI_CHECK << ASISetStartPos(info.CameraID, roi.x(), roi.y()) << "Set ROI position";
    ASI_CHECK << ASIStartVideoCapture(info.CameraID) << "Start video capture";
    buffer.resize(calcBufferSize());
    qDebug() << "Imaging started: imageFormat=" << format << ", roi: " << roi << ", bin: " << bin;
}

void ASIImagingWorker::start()
{
}

bool ASIImagingWorker::shoot(const ImageHandlerPtr& imageHandler)
{
  try {
    ASI_CHECK << ASIGetVideoData(info.CameraID, buffer.data(), buffer.size(), 100000) << "Capture frame";
        cv::Mat image( {roi.width(), roi.height()}, getCVImageType(), buffer.data());
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
    ASI_CHECK << ASIStopVideoCapture(info.CameraID) << "Stop capture";
    qDebug() << "Imaging stopped.";
}

size_t ASIImagingWorker::calcBufferSize()
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

int ASIImagingWorker::getCVImageType()
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

