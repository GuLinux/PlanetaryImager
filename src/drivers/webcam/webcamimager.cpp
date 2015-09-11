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

#include "webcamimager_p.h"
using namespace std;


WebcamImager::Private::Private(const QString &name, int index, const ImageHandlerPtr &handler, WebcamImager *q)
    : name {name}, index {index}, handler {handler}, q {q} {
        populate_rules();
}


QString fourcc2s(int32_t _4cc)
{
    char data[5] { _4cc & 0xff, _4cc >> 8 & 0xff, _4cc >> 0x10 & 0xff, _4cc >> 0x18 & 0xff, '\0' };
    return {data};
}

WebcamImager::WebcamImager(const QString &name, int index, const ImageHandlerPtr &handler)
    : dptr(name, index, handler, this)
{
    d->capture = make_shared<cv::VideoCapture> (d->index);
    if (!d->capture->isOpened()) {
        qDebug() << "error opening device";
    }
    d->read_v4l2_parameters();
    qDebug() << "driver:" << d->driver << ", bus:" << d->bus << ", devicename:" << d->cameraname;
    if(!d->resolutions.empty()) {
        d->capture->set(CV_CAP_PROP_FRAME_WIDTH, d->resolutions.last().width);
        d->capture->set(CV_CAP_PROP_FRAME_HEIGHT, d->resolutions.last().height);
    }
}


QDebug operator << (QDebug dbg, const v4l2_queryctrl &q)
{
    static QMap<int, QString> types {
        { V4L2_CTRL_TYPE_INTEGER, "V4L2_CTRL_TYPE_INTEGER" },
        { V4L2_CTRL_TYPE_BOOLEAN, "V4L2_CTRL_TYPE_BOOLEAN" },
        { V4L2_CTRL_TYPE_MENU, "V4L2_CTRL_TYPE_MENU" },
        { V4L2_CTRL_TYPE_BUTTON, "V4L2_CTRL_TYPE_BUTTON" },
        { V4L2_CTRL_TYPE_INTEGER64, "V4L2_CTRL_TYPE_INTEGER64" },
        { V4L2_CTRL_TYPE_CTRL_CLASS, "V4L2_CTRL_TYPE_CTRL_CLASS" },
        { V4L2_CTRL_TYPE_STRING, "V4L2_CTRL_TYPE_STRING" },
        { V4L2_CTRL_TYPE_BITMASK, "V4L2_CTRL_TYPE_BITMASK" },
        { V4L2_CTRL_TYPE_INTEGER_MENU, "V4L2_CTRL_TYPE_INTEGER_MENU" },
        { V4L2_CTRL_COMPOUND_TYPES, "V4L2_CTRL_COMPOUND_TYPES" },
        { V4L2_CTRL_TYPE_U8, "V4L2_CTRL_TYPE_U8" },
        { V4L2_CTRL_TYPE_U16, "V4L2_CTRL_TYPE_U16" },
        { V4L2_CTRL_TYPE_U32, "V4L2_CTRL_TYPE_U32" },
    };
    dbg.nospace() << "v4l2_queryctrl{";
    dbg << "id=" << q.id << ", type=" << types[q.type] << ", name=" << (char *)(q.name) << ", default_value=" << q.default_value
        << ", min=" << q.minimum << ", max=" << q.maximum << ", step=" << q.step;
    dbg << "}";
    return dbg.space();
}


void WebcamImager::Private::read_v4l2_parameters()
{
    auto dev_name = "/dev/video%1"_q % index;
    v4l_fd = ::open(dev_name.toLatin1(), O_RDWR, O_NONBLOCK, 0);
    if (-1 == v4l_fd) {
        qWarning() << "Cannot open '%1': %2, %3"_q % dev_name % errno % strerror(errno);
        return;
    }
    
    v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(-1 == ioctl(v4l_fd, VIDIOC_G_FMT , &format)) {
        qWarning() << "Unable to query webcam format: " << strerror(errno);
        return;
    } 
    
    v4l2_fmtdesc formats;
    QList<v4l2_fmtdesc> formats_list;
    formats.index = 0;
    formats.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while(0 == ioctl(v4l_fd, VIDIOC_ENUM_FMT, &formats)) {
        qDebug() << "found format: " << formats.index << (char*)formats.description;
        formats_list.push_back(formats);
        formats.index++;
    }

    
    
    v4l2_frmsizeenum frmsize;
    frmsize.pixel_format = format.fmt.pix.pixelformat;
    frmsize.index = 0;
    while (ioctl(v4l_fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {
        Resolution resolution;
        if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            resolution = {frmsize.discrete.width , frmsize.discrete.height};
        } else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
            resolution = {frmsize.stepwise.max_width , frmsize.stepwise.max_height};
        }
        if (resolution && ! resolutions.contains(resolution))
            resolutions.push_back(resolution);
        frmsize.index++;
    }
    
    qSort(resolutions);
    qDebug() << "available resolutions: " << resolutions;
    

    v4l2_capability cap;
    if(-1 == ioctl(v4l_fd, VIDIOC_QUERYCAP, &cap)) {
        qWarning() << "Unable to query webcam capabilities: " << strerror(errno);
        return;
    }
    driver = {(char*) cap.driver};
    bus = {(char*) cap.bus_info};
    cameraname = {(char*) cap.card};
    
    /*
    v4l2_fmtdesc formats;
    QList<v4l2_fmtdesc> formats_list;
    formats.index = 0;
    formats.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while(0 == ioctl(v4l_fd, VIDIOC_ENUM_FMT, &formats)) {
        qDebug() << "found format: " << formats.index << (char*)formats.description;
        formats_list.push_back(formats);
        formats.index++;
    }
        
    v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(-1 == ioctl(v4l_fd, VIDIOC_G_FMT , &format)) {
        qWarning() << "Unable to query webcam format: " << strerror(errno);
        return;
    } 
    format.fmt.pix.pixelformat = formats_list[1].pixelformat;
    format.fmt.pix.flags= formats_list[1].flags;
    if(-1 == ioctl(v4l_fd, VIDIOC_S_FMT , &format)) {
        qWarning() << "Unable to query webcam format: " << strerror(errno);
        return;
    } 
    if(-1 == ioctl(v4l_fd, VIDIOC_G_FMT , &format)) {
        qWarning() << "Unable to query webcam format: " << strerror(errno);
        return;
    } 

    
    qDebug() << "format: " << format.fmt.pix.pixelformat
        << (char) (format.fmt.pix.pixelformat & 0xff) 
        << (char) (format.fmt.pix.pixelformat >> 8 & 0xff) 
        << (char) (format.fmt.pix.pixelformat >> 0x10 & 0xff) 
        << (char) (format.fmt.pix.pixelformat >> 0x18 & 0xff);
        */
}






WebcamImager::~WebcamImager()
{
    stopLive();
    ::close(d->v4l_fd);
}

Imager::Chip WebcamImager::chip() const
{
    Chip chip{};
    memset(&chip, -1, sizeof(chip));
    chip.xres = d->resolutions.last().width;
    chip.yres = d->resolutions.last().height;
    return chip;
}

QString WebcamImager::name() const
{
    return d->cameraname;
}

// http://www.linuxtv.org/downloads/legacy/video4linux/API/V4L2_API/spec-single/v4l2.html#control
WebcamImager::Private::V4lSetting WebcamImager::Private::setting(int id)
{
    V4lSetting setting;
    v4l2_queryctrl ctrl{id};
    setting.querycode = ioctl(v4l_fd, VIDIOC_QUERYCTRL, &ctrl);
    if (0 != setting.querycode) {
        return setting;
    }
    setting.disabled = (ctrl.flags & V4L2_CTRL_FLAG_DISABLED);
    if (setting.disabled)
        return setting;
    static QMap<int, Setting::Type> types {
        {V4L2_CTRL_TYPE_INTEGER, Setting::Number},
        {V4L2_CTRL_TYPE_BOOLEAN, Setting::Bool},
        {V4L2_CTRL_TYPE_MENU, Setting::Combo},
    };
    setting.unknown_type = types.count(ctrl.type) == 0;
    if(setting.unknown_type)
        return setting;
    
    v4l2_control control{ctrl.id};
    setting.valuecode = ioctl(v4l_fd, VIDIOC_G_CTRL, &control);
    if (-1 == setting.valuecode) {
        qWarning() << "error on VIDIOC_G_CTRL" << strerror(errno);
        return setting;
    }
    setting.setting = Imager::Setting{ctrl.id, reinterpret_cast<char*>(ctrl.name), ctrl.minimum, ctrl.maximum, ctrl.step, control.value, ctrl.default_value};
    setting.setting.type = types[ctrl.type];
    if(ctrl.type == V4L2_CTRL_TYPE_MENU) {
        v4l2_querymenu menu{ctrl.id};
        for(menu.index = ctrl.minimum; menu.index <= ctrl.maximum; menu.index++) {
            QString value;
            if (0 == ioctl (v4l_fd, VIDIOC_QUERYMENU, &menu)) {
                value = {(char*)menu.name};
            }
            setting.setting.choices.push_back({value, menu.index});
        }
    }
//     qDebug() << "setting: " << setting.setting << "(exposure: " << V4L2_CID_EXPOSURE_ABSOLUTE << ")";
    for(auto rule: setting_rules)
        rule(setting.setting);
    return setting;
}


Imager::Settings WebcamImager::settings() const
{
    Imager::Settings _settings;

    for (int ctrlid = V4L2_CID_BASE; ctrlid < V4L2_CID_LASTP1; ctrlid++) {
        auto v4lsetting = d->setting(ctrlid);
        if(!v4lsetting)
            continue;
        _settings.push_back(v4lsetting.setting);
    }
    
    for (int ctrlid = V4L2_CID_PRIVATE_BASE;; ctrlid++) {
        auto v4lsetting = d->setting(ctrlid);
        if (!v4lsetting && errno == EINVAL)
                break;
        _settings.push_back(v4lsetting.setting);
    }
    
    int ctrlid = V4L2_CTRL_FLAG_NEXT_CTRL;
    Private::V4lSetting v4lsetting;
    do {
        v4lsetting = d->setting(ctrlid);
        if(v4lsetting)
            _settings.push_back(v4lsetting.setting);
        ctrlid = v4lsetting.setting.id | V4L2_CTRL_FLAG_NEXT_CTRL;
    } while (v4lsetting.querycode != -1);
    std::sort(begin(_settings), end(_settings), [](const Setting &a, const Setting &b){ return a.id < b.id; });
    _settings.erase(std::unique(begin(_settings), end(_settings), [](const Setting &a, const Setting &b){ return a.id == b.id; }), end(_settings));
    return _settings;
}

void WebcamImager::setSetting(const Imager::Setting &setting)
{
    v4l2_control control{setting.id, setting.value};
    if (-1 == ioctl (d->v4l_fd, VIDIOC_S_CTRL, &control)) {
        qWarning() << "Error setting control " << setting.id << setting.name << " to " << setting.value << strerror(errno);
    }
//     emit changed(d->setting(setting.id).setting);
}

void WebcamImager::startLive()
{
    d->live = true;
    d->future = QtConcurrent::run([ = ] {
        fps_counter fps([ = ](double rate) {
            emit this->fps(rate);
        }, fps_counter::Mode::Elapsed);

        while (d->live) {
            cv::Mat frame;
            *d->capture >> frame;
            d->handler->handle(frame);
            ++fps;
        }
    });
}

void WebcamImager::stopLive()
{
    d->live = false;
    if(d->future.isRunning())
        d->future.waitForFinished();
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
