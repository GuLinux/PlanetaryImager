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


FC2ImagerWorker::FC2ImagerWorker(
    FlyCapture2::Camera &_camera,
    FC2VideoMode vidMode,
    FlyCapture2::FrameRate frameRate,
    FlyCapture2::PixelFormat pixFmt,
    /// Must be already validated; also used as the initial frame size for Format7 modes
    const QRect &roi)
: camera(_camera)
{
    frameInfo.initialized = false;

    if (vidMode.isFormat7())
    {
        FlyCapture2::Format7ImageSettings fmt7settings;

        fmt7settings.mode = (FlyCapture2::Mode)vidMode;
        fmt7settings.offsetX = roi.left();
        fmt7settings.offsetY = roi.top();
        fmt7settings.width = roi.width();
        fmt7settings.height = roi.height();
        fmt7settings.pixelFormat = pixFmt;

        FC2_CHECK << camera.SetFormat7Configuration(&fmt7settings, 100.0f).GetType()
                  << "Camera::SetFormat7Configuration";
    }
    else
    {
        std::cout << "Passing " << (int)vidMode << ", " << (int)frameRate << std::endl;  //TESTING ################
        FC2_CHECK << camera.SetVideoModeAndFrameRate((FlyCapture2::VideoMode)vidMode, frameRate).GetType()
                  << "Camera::SetVideoModeAndFrameRate";
    }

    FC2_CHECK << camera.StartCapture(nullptr, nullptr).GetType()
              << "Camera::StartCapture";
}

FC2ImagerWorker::~FC2ImagerWorker()
{
    FC2_CHECK << camera.StopCapture().GetType()
              << "Camera::StopCapture";
}

void FC2ImagerWorker::initFrameInfo()
{
    const auto pixFmt = image.GetPixelFormat();

    switch (pixFmt)
    {
    case FlyCapture2::PIXEL_FORMAT_MONO8:
    case FlyCapture2::PIXEL_FORMAT_MONO12:
    case FlyCapture2::PIXEL_FORMAT_MONO16:
    case FlyCapture2::PIXEL_FORMAT_S_MONO16:
        frameInfo.colorFormat = Frame::ColorFormat::Mono; break;

    // YUV formats will be converted to RGB before returning the frame
    case FlyCapture2::PIXEL_FORMAT_411YUV8:
    case FlyCapture2::PIXEL_FORMAT_422YUV8:
    case FlyCapture2::PIXEL_FORMAT_444YUV8:
    case FlyCapture2::PIXEL_FORMAT_RGB8:
    case FlyCapture2::PIXEL_FORMAT_RGB16:
        frameInfo.colorFormat = Frame::ColorFormat::RGB; break;

    case FlyCapture2::PIXEL_FORMAT_RAW8:
    case FlyCapture2::PIXEL_FORMAT_RAW12:
    case FlyCapture2::PIXEL_FORMAT_RAW16:
        switch (image.GetBayerTileFormat())
        {
        case FlyCapture2::RGGB: frameInfo.colorFormat = Frame::ColorFormat::Bayer_RGGB; break;
        case FlyCapture2::BGGR: frameInfo.colorFormat = Frame::ColorFormat::Bayer_BGGR; break;
        case FlyCapture2::GBRG: frameInfo.colorFormat = Frame::ColorFormat::Bayer_GBRG; break;
        case FlyCapture2::GRBG: frameInfo.colorFormat = Frame::ColorFormat::Bayer_GRBG; break;
        }
        break;
    }

    frameInfo.bitsPerChannel = image.GetBitsPerPixel();

    switch (pixFmt)
    {
    case FlyCapture2::PIXEL_FORMAT_411YUV8: frameInfo.srcBytesPerLine = (3 * image.GetCols() + 1) / 2; break;
    case FlyCapture2::PIXEL_FORMAT_422YUV8: frameInfo.srcBytesPerLine = 2 * image.GetCols(); break;
    case FlyCapture2::PIXEL_FORMAT_444YUV8: frameInfo.srcBytesPerLine = 3 * image.GetCols(); break;

    case FlyCapture2::PIXEL_FORMAT_RGB8:    frameInfo.srcBytesPerLine = 3 * image.GetCols(); break;
    case FlyCapture2::PIXEL_FORMAT_RGB16:   frameInfo.srcBytesPerLine = 6 * image.GetCols(); break;

    default: frameInfo.srcBytesPerLine = 0; break;
    }

    if (pixFmt == FlyCapture2::PIXEL_FORMAT_RGB8 ||
        pixFmt == FlyCapture2::PIXEL_FORMAT_RGB16)
    {
        frameInfo.bitsPerChannel /= 3;
    }
}

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
        qWarning() << "Image consistency error detected, skipping frame";
        badFrameCounter++;
    }

    FC2_CHECK << result.GetType() << "Camera::RetrieveBuffer";

    if (!frameInfo.initialized)
    {
        initFrameInfo();
        frameInfo.initialized = true;

//        if (isYUV(nativeFrame->color_coding))
//        {
//            frameInfo.needsYUVtoRGBconversion = true;
//            conversionBuf.src = std::make_unique<uint8_t[]>(nativeFrame->total_bytes); // Pass 'total_bytes' for simplicity; we may use less
//            conversionBuf.dest = std::make_unique<uint8_t[]>(nativeFrame->size[0] * nativeFrame->size[1] * 3); // 3 bytes/pixel (R, G, B)
//        }
//        else
//            frameInfo.needsYUVtoRGBconversion = false;

    }

    auto frame = std::make_shared<Frame>(frameInfo.bitsPerChannel,
                                         frameInfo.colorFormat,
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
