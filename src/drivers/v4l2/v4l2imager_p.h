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
#include "v4l2formats.h"

#define PIXEL_FORMAT_CONTROL_ID -10
#define RESOLUTIONS_CONTROL_ID -9
#define FPS_CONTROL_ID -8

class V4L2Device;


DPTR_IMPL(V4L2Imager)
{
    class Worker;
    ImageHandler::ptr handler;
    const QString device_path;
    V4L2Imager *q;
    
    V4L2Device::ptr device;
    ImagerThread::ptr imager_thread;
    
    V4L2Formats::ptr v4l2formats;
    QList<V4L2Formats::Resolution::ptr> resolutions;

    void open_camera();
    QList<V4L2Control::ptr> controls;
    QString driver, bus, cameraname;
    QString dev_name;
    QList<V4L2Control::Fix> control_fixes;
    void populate_control_fixes();
    void find_controls();
};



#endif
