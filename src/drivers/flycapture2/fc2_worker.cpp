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

#include <QRect>

#include "fc2_exception.h"
#include "fc2_worker.h"
#include "commons/frame.h"


// Captured frames may be "inconsistent" (have damaged contents), e.g. sometimes when using GigE cameras
// on Linux with its network stack (instead of PGR's Ethernet Filter Driver under Windows). Allow a few of them
// before reporting error.
constexpr int MAX_NUM_INCONSISTENT_FRAMES_TO_SKIP = 15;


FC2ImagerWorker::FC2ImagerWorker(
    fc2Context _context,
    FC2VideoMode vidMode,
    fc2FrameRate frameRate,
    fc2PixelFormat pixFmt,
    /// Must be already validated; also used as the initial frame size for Format7 modes
    const QRect &roi)
: context(_context)
{
    frameInfo.initialized = false;

    if (vidMode.isFormat7())
    {
        fc2Format7ImageSettings fmt7settings;
        memset(&fmt7settings, 0, sizeof(fmt7settings));
        fmt7settings.mode = (fc2Mode)vidMode;
        fmt7settings.offsetX = roi.left();
        fmt7settings.offsetY = roi.top();
        fmt7settings.width = roi.width();
        fmt7settings.height = roi.height();
        fmt7settings.pixelFormat = pixFmt;

        FC2_CHECK << fc2SetFormat7Configuration(context, &fmt7settings, 100.0f)
                  << "fc2SetFormat7Configuration";
    }
    else
    {
        FC2_CHECK << fc2SetVideoModeAndFrameRate(context, (fc2VideoMode)vidMode, frameRate)
                  << "fc2SetVideoModeAndFrameRate";
    }

    FC2_CHECK << fc2StartCapture(context)
              << "fc2StartCapture";

    memset(&image, 0, sizeof(image));
    FC2_CHECK << fc2CreateImage(&image)
              << "fc2CreateImage";
}

FC2ImagerWorker::~FC2ImagerWorker()
{
    FC2_CHECK << fc2StopCapture(context)
              << "fc2StopCapture";

    FC2_CHECK << fc2DestroyImage(&image)
              << "fc2DestroyImage";
}

void FC2ImagerWorker::initFrameInfo()
{
    switch (image.format)
    {
    case FC2_PIXEL_FORMAT_MONO8:
    case FC2_PIXEL_FORMAT_MONO12:
    case FC2_PIXEL_FORMAT_MONO16:
    case FC2_PIXEL_FORMAT_S_MONO16:
        frameInfo.colorFormat = Frame::ColorFormat::Mono; break;

    // YUV formats will be converted to RGB before returning the frame
    case FC2_PIXEL_FORMAT_411YUV8:
    case FC2_PIXEL_FORMAT_422YUV8:
    case FC2_PIXEL_FORMAT_444YUV8:
    case FC2_PIXEL_FORMAT_RGB8:
    case FC2_PIXEL_FORMAT_RGB16:
        frameInfo.colorFormat = Frame::ColorFormat::RGB; break;

    case FC2_PIXEL_FORMAT_RAW8:
    case FC2_PIXEL_FORMAT_RAW12:
    case FC2_PIXEL_FORMAT_RAW16:
        switch (image.bayerFormat)
        {
        case FC2_BT_RGGB: frameInfo.colorFormat = Frame::ColorFormat::Bayer_RGGB; break;
        case FC2_BT_BGGR: frameInfo.colorFormat = Frame::ColorFormat::Bayer_BGGR; break;
        case FC2_BT_GBRG: frameInfo.colorFormat = Frame::ColorFormat::Bayer_GBRG; break;
        case FC2_BT_GRBG: frameInfo.colorFormat = Frame::ColorFormat::Bayer_GRBG; break;
        }
        break;
    }

    unsigned int bpch;
    FC2_CHECK << fc2DetermineBitsPerPixel(image.format, &bpch)
              << "fc2DetermineBitsPerPixel";
    frameInfo.bitsPerChannel = bpch;

    switch (image.format)
    {
    case FC2_PIXEL_FORMAT_411YUV8: frameInfo.srcBytesPerLine = (3 * image.cols + 1) / 2; break;
    case FC2_PIXEL_FORMAT_422YUV8: frameInfo.srcBytesPerLine = 2 * image.cols; break;
    case FC2_PIXEL_FORMAT_444YUV8: frameInfo.srcBytesPerLine = 3 * image.cols; break;

    case FC2_PIXEL_FORMAT_RGB8:    frameInfo.srcBytesPerLine = 3 * image.cols; break;
    case FC2_PIXEL_FORMAT_RGB16:   frameInfo.srcBytesPerLine = 6 * image.cols; break;

    default: frameInfo.srcBytesPerLine = 0; break;
    }

    if (image.format == FC2_PIXEL_FORMAT_RGB8 ||
        image.format == FC2_PIXEL_FORMAT_RGB16)
    {
        frameInfo.bitsPerChannel /= 3;
    }
}

FramePtr FC2ImagerWorker::shoot()
{
//    //TODO: fail gracefully if cannot capture

    int badFrameCounter = 0;
    fc2Error result;
    while (badFrameCounter < MAX_NUM_INCONSISTENT_FRAMES_TO_SKIP &&
           (result = fc2RetrieveBuffer(context, &image)) == FC2_ERROR_IMAGE_CONSISTENCY_ERROR)
    {
        qWarning() << "Image consistency error detected, skipping frame";
        badFrameCounter++;
    }

    FC2_CHECK << result << "fc2RetrieveBuffer";

    if (!frameInfo.initialized)
    {
        initFrameInfo();
        frameInfo.initialized = true;
    }

    auto frame = std::make_shared<Frame>(frameInfo.bitsPerChannel,
                                         frameInfo.colorFormat,
                                         QSize(image.cols, image.rows),
                                         Frame::ByteOrder::LittleEndian);

    const uint8_t *srcLine = image.pData;
    const ptrdiff_t srcStride = image.stride;

    uint8_t *destLine = frame->mat().data;
    const size_t destStride = frame->mat().step[0];
    size_t numDestCopyBytes; // Number of bytes per line to copy into 'frame'

    numDestCopyBytes = std::min(destStride, (size_t)srcStride);

    for (int y = 0; y < frame->mat().rows; y++)
    {
        memcpy(destLine, srcLine, numDestCopyBytes);
        srcLine += srcStride;
        destLine += destStride;
    }

    return frame;
}
