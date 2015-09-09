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

#include "v4l2imager_p.h"
using namespace std;
using namespace GuLinux;

QString FOURCC2QS(int32_t _4cc)
{
    auto get_byte = [=](int b) { return static_cast<char>( _4cc >> b & 0xff ); };
    char data[5] { get_byte(0), get_byte(8), get_byte(0x10), get_byte(0x18), '\0' };
    return {data};
}

V4L2Imager::Private::Private(const ImageHandlerPtr &handler, V4L2Imager *q) : handler{handler}, q(q)
{
}

V4L2Imager::V4L2Imager(const QString &name, int index, const ImageHandlerPtr &handler)
    : dptr(handler, this)
{
    auto dev_name = "/dev/video%1"_q % index;
    d->v4l_fd = ::open(dev_name.toLatin1(), O_RDWR, O_NONBLOCK, 0);
    if (-1 == d->v4l_fd) {
        qWarning() << "Cannot open '%1': %2, %3"_q % dev_name % errno % strerror(errno);
        return;
    }
    v4l2_capability cap;
    if(-1 == Private::ioctl(d->v4l_fd, VIDIOC_QUERYCAP, &cap)) {
        qWarning() << "Unable to query webcam capabilities: " << strerror(errno);
        return;
    }
    d->driver = {(char*) cap.driver};
    d->bus = {(char*) cap.bus_info};
    d->cameraname = {(char*) cap.card};
}

V4L2Imager::~V4L2Imager()
{
    stopLive();
    ::close(d->v4l_fd);
}

Imager::Chip V4L2Imager::chip() const
{
    return {};
}

QString V4L2Imager::name() const
{
    return d->cameraname;
}


Imager::Settings V4L2Imager::settings() const
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


V4L2Imager::Private::V4lSetting V4L2Imager::Private::setting(int id)
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
//     for(auto rule: setting_rules)
//         rule(setting.setting);
    return setting;
}

void V4L2Imager::setSetting(const Setting &setting)
{

}

void V4L2Imager::startLive()
{
    d->live = true;
    Thread::Run<void>{
        [=]{
            fps_counter fps([ = ](double rate) {
                emit this->fps(rate);
            }, fps_counter::Mode::Elapsed);
            while (d->live) {
                cv::Mat frame;
                d->handler->handle(frame);
                ++fps;
            }
        }, 
        [=]{ d->live_thread = nullptr; }, 
        [=](Thread *thread) { d->live_thread = thread; d->live = true; }
    };
}

void V4L2Imager::stopLive()
{
    d->live = false;
    d->live_thread->wait();
}



int V4L2Imager::Private::ioctl(int fh, int request, void* arg)
{
    int r;
    do {
        r = ::ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}



v4l2_format V4L2Imager::Private::query_format() const
{
    v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(-1 == ioctl(v4l_fd, VIDIOC_G_FMT , &format)) {
        qWarning() << "Unable to query webcam format: " << strerror(errno);
        return {};
    } 
    return format;
}


QList< v4l2_frmsizeenum > V4L2Imager::Private::resolutions() const
{
    auto format = query_format();
    QList< v4l2_frmsizeenum > values;
    v4l2_frmsizeenum frmsize;
    frmsize.pixel_format = format.fmt.pix.pixelformat;
    frmsize.index = 0;
    while (ioctl(v4l_fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0) {
        values.push_back(frmsize);
        frmsize.index++;
    }
    return values;
}

