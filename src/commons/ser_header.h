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
#ifndef _SER_HEADER_H
#define _SER_HEADER_H
#include <stdint.h>
#include "frame.h"
#include <QDateTime>
#include <QtEndian>
// TODO: stop using a packed struct, and use accessors + QtEndian helpers for better compat

typedef uint64_t SER_Timestamp;
struct SER_Header {
    char fileId[14] = {'L', 'U', 'C', 'A', 'M', '-', 'R','E','C','O','R','D','E','R'};
    qint32 luId = 0;
    enum ColorId {
        MONO = 0,
        BAYER_RGGB = 8,
        BAYER_GRBG = 9,
        BAYER_GBRG = 10,
        BAYER_BGGR = 11,
        BAYER_CYYM = 16,
        BAYER_YCMY = 17,
        BAYER_YMCY = 18,
        BAYER_MYYC = 19,
        RGB = 100,
        BGR = 101,
    };
    qint32 colorId = MONO;
    enum Endian { BigEndian = 0, LittleEndian = 1 };
    qint32 endian = LittleEndian;
    quint32 imageWidth = 0;
    quint32 imageHeight = 0;
    quint32 pixelDepth = 0;
    quint32 frames = 0;
    char observer[40] = {};
    char camera[40] = {};
    char telescope[40] = {};
    SER_Timestamp datetime = 0;
    SER_Timestamp datetime_utc = 0;
    std::size_t frame_size() const;
    int channels() const;
    int bytesPerPixel() const;
    Frame::ColorFormat frame_color_format() const;
    void set_color_format(const Frame::ColorFormat &format);
    static SER_Timestamp timestamp(const QDateTime &datetime);
    static QDateTime qdatetime(const SER_Timestamp &timestamp);
} __attribute__ ((__packed__));

static_assert(sizeof(SER_Header) == 178, "Wrong SER_Header size");
#endif
