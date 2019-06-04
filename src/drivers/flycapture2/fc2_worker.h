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

#include <C/FlyCapture2_C.h>
#include <commons/utils.h>
#include <drivers/imagerthread.h>


/// Contains either a value from fc2VideoMode, or a value from fc2Mode (i.e. a Format7 mode) + FMT7_BASE
class FC2VideoMode
{
    static constexpr int FMT7_BASE = FC2_NUM_VIDEOMODES + 1;

    int mode;

public:

    FC2VideoMode(): mode(-1) { }
    FC2VideoMode(const FC2VideoMode &) = default;
    FC2VideoMode(int _mode): mode(_mode) { }
    FC2VideoMode(fc2VideoMode vidMode): mode(vidMode) { }
    FC2VideoMode(fc2Mode fmt7Mode): mode(fmt7Mode + FMT7_BASE) { }

    explicit operator fc2VideoMode() const
    {
        if (isFormat7())
            throw std::runtime_error("Cannot treat Format7 mode as non-Format7");
        else
            return (fc2VideoMode)mode;
    }

    explicit operator fc2Mode() const
    {
        if (!isFormat7())
            throw std::runtime_error("Cannot treat non-Format7 mode as Format7");
        else
            return (fc2Mode)(mode - FMT7_BASE);
    }

    explicit operator int() const { return mode; }

    bool isFormat7() const { return mode >= FMT7_BASE + FC2_MODE_0; }
};


class FC2ImagerWorker: public ImagerThread::Worker
{
    fc2Context context;

    fc2Image image;

    struct
    {
        bool initialized; ///< If 'false', the remaining fields have not been set yet

        Frame::ColorFormat colorFormat;
        uint8_t bitsPerChannel;
        bool needsYUVtoRGBconversion;
        size_t srcBytesPerLine; ///< Set only for YUV formats
    } frameInfo;

    void initFrameInfo();

    LOG_C_SCOPE(FC2ImagerWorker);

public:

    FC2ImagerWorker(fc2Context _context,
                    FC2VideoMode vidMode,
                    fc2FrameRate frameRate,
                    fc2PixelFormat pixFmt,
                    /// Must be already validated; also used as the initial frame size for Format7 modes
                    const QRect &roi);

    FramePtr shoot() override;

    virtual ~FC2ImagerWorker();
};


#endif // FC2_IMAGER_WORKER_H
