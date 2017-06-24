/*
 * Copyright (C) 2017 Filip Szczerek <ga.software@yahoo.com>
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

#include <thread> //TESTING ########
#include <QRect>

#include "fc2_exception.h"
#include "fc2_worker.h"


// Captured frames may be "inconsistent" (have damaged contents), e.g. sometimes when using GigE cameras
// on Linux with its network stack (instead of PGR's Ethernet Filter Driver under Windows). Allow a few of them
// before reporting error.
constexpr int MAX_NUM_INCONSISTENT_FRAMES_TO_SKIP = 15;


FC2ImagerWorker::FC2ImagerWorker(FlyCapture2::Camera &_camera, //dc1394camera_t *_camera, dc1394video_mode_t _vidMode,
                                   const QRect &roi)
: camera(_camera)
// //camera(_camera), nativeFrame(nullptr), vidMode(_vidMode)
{
//    frameInfo.initialized = false;
//
//    IIDC_CHECK << dc1394_video_set_mode(camera, vidMode)
//               << "Set video mode";
//
    qDebug() << "Requested to set ROI to " << roi.x() << ", " << roi.y() << ", " << roi.width() << ", " << roi.height();

//    setROI(roi);
//
//    IIDC_CHECK << dc1394_capture_setup(camera, NUM_DMA_BUFFERS, DC1394_CAPTURE_FLAGS_DEFAULT)
//               << "Setup capture";
//
//    IIDC_CHECK << dc1394_video_set_transmission(camera, DC1394_ON)
//               << "Start video transmission";


    FC2_CHECK << camera.StartCapture(nullptr, nullptr).GetType()
              << "Camera::StartCapture";
}

FC2ImagerWorker::~FC2ImagerWorker()
{
    FC2_CHECK << camera.StopCapture().GetType()
              << "Camera::StopCapture";
}

//void FC2ImagerWorker::initFrameInfo()
//{
//    switch (nativeFrame->color_coding)
//    {
//    case DC1394_COLOR_CODING_MONO8:
//    case DC1394_COLOR_CODING_MONO16:
//    case DC1394_COLOR_CODING_MONO16S:
//        frameInfo.colorFormat = Frame::ColorFormat::Mono; break;
//
//    // YUV formats will be converted to RGB before returning the frame
//    case DC1394_COLOR_CODING_YUV411:
//    case DC1394_COLOR_CODING_YUV422:
//    case DC1394_COLOR_CODING_YUV444:
//    case DC1394_COLOR_CODING_RGB8:
//    case DC1394_COLOR_CODING_RGB16:
//    case DC1394_COLOR_CODING_RGB16S:
//        frameInfo.colorFormat = Frame::ColorFormat::RGB; break;
//
//    case DC1394_COLOR_CODING_RAW8:
//    case DC1394_COLOR_CODING_RAW16:
//        switch (nativeFrame->color_filter)
//        {
//        case DC1394_COLOR_FILTER_RGGB: frameInfo.colorFormat = Frame::ColorFormat::Bayer_RGGB; break;
//        case DC1394_COLOR_FILTER_BGGR: frameInfo.colorFormat = Frame::ColorFormat::Bayer_BGGR; break;
//        case DC1394_COLOR_FILTER_GBRG: frameInfo.colorFormat = Frame::ColorFormat::Bayer_GBRG; break;
//        case DC1394_COLOR_FILTER_GRBG: frameInfo.colorFormat = Frame::ColorFormat::Bayer_GRBG; break;
//        }
//        break;
//    }
//
//    frameInfo.bitsPerChannel = nativeFrame->data_depth;
//    frameInfo.byteOrder = (nativeFrame->little_endian ? Frame::ByteOrder::LittleEndian : Frame::ByteOrder::BigEndian);
//
//    switch (nativeFrame->color_coding)
//    {
//    case DC1394_COLOR_CODING_YUV411: frameInfo.srcBytesPerLine = (3 * nativeFrame->size[0] + 1) / 2; break;
//    case DC1394_COLOR_CODING_YUV422: frameInfo.srcBytesPerLine = 2 * nativeFrame->size[0]; break;
//    case DC1394_COLOR_CODING_YUV444: frameInfo.srcBytesPerLine = 3 * nativeFrame->size[0]; break;
//
//    default: frameInfo.srcBytesPerLine = 0; break;
//    }
//}

//static bool isYUV(dc1394color_coding_t colorCoding)
//{
//    return colorCoding == DC1394_COLOR_CODING_YUV411 ||
//           colorCoding == DC1394_COLOR_CODING_YUV422 ||
//           colorCoding == DC1394_COLOR_CODING_YUV444;
//}

Frame::ptr FC2ImagerWorker::shoot()
{
//    //TODO: fail gracefully if cannot capture

    int badFrameCounter = 0;
    FlyCapture2::Error result;
    while (badFrameCounter < MAX_NUM_INCONSISTENT_FRAMES_TO_SKIP &&
           (result = camera.RetrieveBuffer(&image)).GetType() == FlyCapture2::PGRERROR_IMAGE_CONSISTENCY_ERROR)
    {
        badFrameCounter++;
    }

    FC2_CHECK << result.GetType() << "Camera::RetrieveBuffer";

    auto frame = std::make_shared<Frame>(image.GetBitsPerPixel(),
                                         Frame::ColorFormat::Mono,
                                         QSize(image.GetCols(), image.GetRows()),
                                         Frame::ByteOrder::LittleEndian);

    const uint8_t *srcLine = image.GetData();
    const ptrdiff_t srcStride = image.GetStride();

    uint8_t *destLine = frame->mat().data;
    const size_t destStride = frame->mat().step[0];
    size_t numDestCopyBytes; // Number of bytes per line to copy into 'frame'

    //...

    numDestCopyBytes = std::min(destStride, (size_t)srcStride);

    for (int y = 0; y < frame->mat().rows; y++)
    {
        memcpy(destLine, srcLine, numDestCopyBytes);
        srcLine += srcStride;
        destLine += destStride;
    }

    return frame;
}

//void FC2ImagerWorker::setROI(const QRect &roi)
//{
//    if (dc1394_is_video_mode_scalable(vidMode))
//    {
//        dc1394color_coding_t colorcd;
//        IIDC_CHECK << dc1394_get_color_coding_from_video_mode(camera, vidMode, &colorcd)
//                   << "Get color coding from video mode";
//
//        IIDC_CHECK << dc1394_format7_set_roi(camera, vidMode, colorcd, DC1394_USE_MAX_AVAIL,
//                                             roi.x(), roi.y(), roi.width(), roi.height())
//                   << "Set ROI";
//    }
//}
