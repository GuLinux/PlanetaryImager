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
#ifndef PLANETARYIMAGER_V4L2_UTILS_H
#define PLANETARYIMAGER_V4L2_UTILS_H
#include <QDebug>
#include <linux/videodev2.h>

inline QString FOURCC2QS(int32_t _4cc)
{
    auto get_byte = [=](int b) { return static_cast<char>( _4cc >> b & 0xff ); };
    char data[5] { get_byte(0), get_byte(8), get_byte(0x10), get_byte(0x18), '\0' };
    return {data};
}



inline QDebug operator<<(QDebug dbg, v4l2_fract frac) {
    dbg.nospace() << frac.numerator << "/" << frac.denominator;
    return dbg.space();
};


inline QDebug operator<<(QDebug dbg, const v4l2_frmivalenum &fps_s) {
    dbg.nospace() << "v4l2_frmivalenum{ index=" << fps_s.index << ", " << fps_s.width << "x" << fps_s.height << ", 4cc=" << FOURCC2QS(fps_s.pixel_format);
    if(fps_s.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
        dbg << "discrete: " << fps_s.discrete;
    }
    if(fps_s.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
        dbg << "stepwise: min=" << fps_s.stepwise.min << ", max=" << fps_s.stepwise.max << ", step=" << fps_s.stepwise.step;
    }
    if(fps_s.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
        dbg << "continuous" << fps_s.stepwise.min << ", max=" << fps_s.stepwise.max << ", step=" << fps_s.stepwise.step;
    }
    dbg << " }";
    return dbg.space();
}
#endif
