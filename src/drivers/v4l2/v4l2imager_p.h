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
#ifndef V4L2_IMAGER_P_H
#define V4L2_IMAGER_P_H


#include "v4l2imager.h"
#include <QDebug>
#include "commons/fps_counter.h"
#include <linux/videodev2.h>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "Qt/strings.h"
#include "Qt/functional.h"
#include <sys/mman.h>
#include "drivers/imagerthread.h"
#include "v4l2buffer.h"
#include "v4l2control.h"
#include "v4l2utils.h"

#define PIXEL_FORMAT_CONTROL_ID -10
#define RESOLUTIONS_CONTROL_ID -9
#define FPS_CONTROL_ID -8

class V4L2Device;


DPTR_IMPL(V4L2Imager)
{
    class Worker;
    ImageHandlerPtr handler;
    const QString device_path;
    V4L2Imager *q;
    
    V4L2Device::ptr device;
    ImagerThread::ptr imager_thread;

    QList<v4l2_fmtdesc> formats() const;
    v4l2_format query_format() const;
    QList<v4l2_frmsizeenum> resolutions(const v4l2_format &format) const;
    void adjust_framerate(const v4l2_format &format) const;
    void open_camera();
    QList<V4L2Control::ptr> controls;
    QString driver, bus, cameraname;
    QString dev_name;
    QList<V4L2Control::Fix> control_fixes;
    void populate_control_fixes();
    void find_controls();
};



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
