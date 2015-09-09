/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <marco.gulino@bhuman.it>
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

#ifndef _WEBCAM_IMAGER_P_H
#define _WEBCAM_IMAGER_P_H

#include "webcamimager.h"
#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include <linux/videodev2.h>
#include "Qt/strings.h"
#include "fps_counter.h"

#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>


class WebcamImager::Private
{
public:
    Private(const QString &name, int index, const ImageHandlerPtr &handler, WebcamImager *q);
    const QString name;
    int index;
    ImageHandlerPtr handler;
    bool live = false;
    std::shared_ptr<cv::VideoCapture> capture;
    void read_v4l2_parameters();
    struct Resolution {
        int width, height;
        bool operator< (const Resolution &other) const {
            return width * height < other.width * other.height;
        };
        bool operator== (const Resolution &other) const {
            return width == other.width && height == other.height;
        }
        operator bool() const {
            return width > 0 && height > 0;
        }
        operator QString() const {
            return "%1x%2"_q % width % height;
        }
        friend QDebug operator<< (QDebug d, const WebcamImager::Private::Resolution &r) {
            d.nospace() << "{" << r.operator QString() << "}";
            return d.space();
        }
    };
    struct V4lSetting {
        Imager::Setting setting;
        int querycode;
        int valuecode;
        bool disabled;
        bool unknown_type;
        operator bool() const { return querycode != -1 && valuecode != -1 && !disabled && !unknown_type; }
    };
    V4lSetting setting(int id);
    QList<Resolution> resolutions;
    QFuture<void> future;
    int v4l_fd;
    QString driver, bus, cameraname;
    typedef std::function<void(Setting &)> SettingRule;
    QList<SettingRule> setting_rules;
    void populate_rules();
private:
    WebcamImager *q;
};
#endif