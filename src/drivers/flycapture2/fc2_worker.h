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

#ifndef FC2_IMAGER_WORKER_H
#define FC2_IMAGER_WORKER_H

#include <commons/utils.h>
//#include <dc1394/dc1394.h>
#include <drivers/imagerthread.h>
#include <Camera.h>
#include <Image.h>


/// Contains either a value from FlyCapture2::VideoMode, or a value from FlyCapture2::Mode (i.e. a Format7 mode) + FMT7_BASE
class FC2VideoMode
{
    static constexpr int FMT7_BASE = FlyCapture2::VideoMode::NUM_VIDEOMODES + 1;

    int mode;

public:

    FC2VideoMode(): mode(-1) { }
    FC2VideoMode(const FC2VideoMode &) = default;
    FC2VideoMode(int _mode): mode(_mode) { }
    FC2VideoMode(FlyCapture2::VideoMode vidMode): mode(vidMode) { }
    FC2VideoMode(FlyCapture2::Mode fmt7Mode): mode(fmt7Mode + FMT7_BASE) { }

    explicit operator FlyCapture2::VideoMode() const
    {
        if (isFormat7())
            throw std::runtime_error("Cannot treat Format7 mode as non-Format7");
        else
            return (FlyCapture2::VideoMode)mode;
    }

    explicit operator FlyCapture2::Mode() const
    {
        if (!isFormat7())
            throw std::runtime_error("Cannot treat non-Format7 mode as Format7");
        else
            return (FlyCapture2::Mode)(mode - FMT7_BASE);
    }

    explicit operator int()                    const { return mode; }

    bool isFormat7() const { return mode >= FMT7_BASE + FlyCapture2::MODE_0; }
};


class FC2ImagerWorker: public ImagerThread::Worker
{
    FlyCapture2::Camera &camera;

    FlyCapture2::Image image;

//    dc1394camera_t *camera;
//    dc1394video_frame_t *nativeFrame; ///< The most recently captured frame
//    dc1394video_mode_t vidMode;
//
    struct
    {
        bool initialized; ///< If 'false', the remaining fields have not been set yet

        Frame::ColorFormat colorFormat;
        uint8_t bitsPerChannel;
        bool needsYUVtoRGBconversion;
        size_t srcBytesPerLine; ///< Set only for YUV formats
    } frameInfo;
//
//    /// Used for YUV->RGB conversion
//    /** Required, because conversion function expects buffers without line padding,
//        which may be present in a dequeued capture buffer. */
//    struct
//    {
//        std::unique_ptr<uint8_t[]> src, dest;
//    } conversionBuf;
//
    void initFrameInfo();
//
    LOG_C_SCOPE(FC2ImagerWorker);

public:

//    static constexpr uint32_t NUM_DMA_BUFFERS = 4;

    FC2ImagerWorker(FlyCapture2::Camera &_camera,
                    FC2VideoMode vidMode,
                    FlyCapture2::FrameRate frameRate,
                    FlyCapture2::PixelFormat pixFmt,
                    /// Must be already validated; also used as the initial frame size for Format7 modes
                    const QRect &roi);

    Frame::ptr shoot() override;

    virtual ~FC2ImagerWorker();
};


#endif // FC2_IMAGER_WORKER_H
