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

V4L2Imager::Private::Private(const ImageHandlerPtr &handler, V4L2Imager *q) : handler{handler}, q(q)
{
}

V4L2Imager::V4L2Imager(const QString &name, int index, const ImageHandlerPtr &handler)
    : dptr(handler, this)
{
    d-> dev_name = "/dev/video%1"_q % index;
  d->open_camera();
}

void V4L2Imager::Private::open_camera() {
    v4l_fd = ::open(dev_name.toLatin1(), O_RDWR, O_NONBLOCK, 0);
    if (-1 == v4l_fd) {
        qWarning() << "Cannot open '%1': %2, %3"_q % dev_name % errno % strerror(errno);
        return;
    }
    v4l2_capability cap;
    if(-1 == Private::ioctl(v4l_fd, VIDIOC_QUERYCAP, &cap)) {
        qWarning() << "Unable to query webcam capabilities: " << strerror(errno);
        return;
    }
    driver = {(char*) cap.driver};
    bus = {(char*) cap.bus_info};
    cameraname = {(char*) cap.card};
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
    
    auto current_format = d->query_format();
    auto formats = d->formats();
    Setting formats_setting{PIXEL_FORMAT_CONTROL_ID, "Format", 0, formats.size()-1, 1, 0, 0, Setting::Combo};
    for(auto format: formats) {
      formats_setting.choices.push_back({FOURCC2QS(format.pixelformat), format.index});
      if(format.pixelformat == current_format.fmt.pix.pixelformat)
	formats_setting.value = format.index;
    }
    _settings.push_back(formats_setting);
    auto resolutions = d->resolutions(current_format);
    Setting resolutions_setting{RESOLUTIONS_CONTROL_ID, "Resolution", 0, resolutions.size()-1, 1, 0, 0, Setting::Combo};
    for(auto resolution: resolutions) {
      resolutions_setting.choices.push_back({"%1x%2"_q % resolution.discrete.width % resolution.discrete.height, resolution.index});
      if(current_format.fmt.pix.width == resolution.discrete.width && current_format.fmt.pix.height == resolution.discrete.height)
	resolutions_setting.value = resolution.index;
    }
    _settings.push_back(resolutions_setting);
    auto framerates = d->framerates(current_format);
    
    std::sort(begin(_settings), end(_settings), [](const Setting &a, const Setting &b){ return a.id < b.id; });
    _settings.erase(std::unique(begin(_settings), end(_settings), [](const Setting &a, const Setting &b){ return a.id == b.id; }), end(_settings));
    return _settings;
}


V4L2Imager::Private::V4lSetting V4L2Imager::Private::setting(int id)
{
    V4lSetting setting;
    v4l2_queryctrl ctrl{id};
    setting.querycode =Private::ioctl(v4l_fd, VIDIOC_QUERYCTRL, &ctrl);
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
    setting.valuecode = Private::ioctl(v4l_fd, VIDIOC_G_CTRL, &control);
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
            if (0 == Private::ioctl (v4l_fd, VIDIOC_QUERYMENU, &menu)) {
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
  auto restart_camera = [=](function<void()> on_restart) {
    auto live_was_started = d->live;
    stopLive();
    ::close(d->v4l_fd);
    d->open_camera();
    on_restart();
    if(live_was_started)
      startLive();
  };
  if(setting.id == PIXEL_FORMAT_CONTROL_ID) {
    restart_camera([=]{
      auto setting_format = d->formats()[setting.value];
      auto current_format = d->query_format();
      qDebug() << "setting format: old=" << FOURCC2QS(current_format.fmt.pix.pixelformat) << ", new=" << FOURCC2QS(setting_format.pixelformat);
      current_format.fmt.pix.pixelformat = setting_format.pixelformat;
      if(-1 == Private::ioctl(d->v4l_fd, VIDIOC_S_FMT , &current_format)) {
	  qWarning() << "Unable to set new webcam format: " << strerror(errno);
	  return;
      }
    });
    return;
  }
  
  if(setting.id == RESOLUTIONS_CONTROL_ID) {
    restart_camera([=]{
      auto current_format = d->query_format();
      auto setting_resolution = d->resolutions(current_format)[setting.value];
      qDebug() << "setting resolution: old=" << current_format.fmt.pix.width << "x" << current_format.fmt.pix.height << ", new=" << setting_resolution.discrete.width << "x" << setting_resolution.discrete.height;
      current_format.fmt.pix.width = setting_resolution.discrete.width;
      current_format.fmt.pix.height = setting_resolution.discrete.height;
      if(-1 == Private::ioctl(d->v4l_fd, VIDIOC_S_FMT , &current_format)) {
	  qWarning() << "Unable to set new webcam resolution: " << strerror(errno);
	  return;
      }
    });
    return;
  }
  
  v4l2_control control{setting.id, setting.value};
  if (-1 == Private::ioctl (d->v4l_fd, VIDIOC_S_CTRL, &control)) {
    qWarning() << "Error setting control " << setting.id << setting.name << " to " << setting.value << strerror(errno);
  }
}

void V4L2Imager::startLive()
{
    d->live = true;
    Thread::Run<void>{
        [=]{
            fps_counter fps([ = ](double rate) {
                emit this->fps(rate);
            }, fps_counter::Mode::Elapsed);
            v4l2_format format = d->query_format();
            qDebug() << "format: " << FOURCC2QS(format.fmt.pix.pixelformat) << ", " << format.fmt.pix.width << "x" << format.fmt.pix.height;
            
            v4l2_requestbuffers bufrequest;
            bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            bufrequest.memory = V4L2_MEMORY_MMAP;
            bufrequest.count = 1;
            if(Private::ioctl(d->v4l_fd, VIDIOC_REQBUFS, &bufrequest) < 0) {
                qDebug() << "error requesting buffers: " << strerror(errno);
                return;
            }
            
            v4l2_buffer bufferinfo;
            memset(&bufferinfo, 0, sizeof(bufferinfo));
            
            bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            bufferinfo.memory = V4L2_MEMORY_MMAP;
            bufferinfo.index = 0;
            
            if(Private::ioctl(d->v4l_fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
                qDebug() << "error allocating buffers: " << strerror(errno);
                return;
            }
            
            char* buffer_start = (char*) mmap(NULL, bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, d->v4l_fd, bufferinfo.m.offset);
            
            if(buffer_start == MAP_FAILED){
                qDebug() << "error memmapping buffer: " << strerror(errno);
                return;
            }
            memset(buffer_start, 0, bufferinfo.length);
            if(Private::ioctl(d->v4l_fd, VIDIOC_QBUF, &bufferinfo) < 0){
                qDebug() << "error queuing buffer: " << strerror(errno);
                return;
            }
            int type = bufferinfo.type;
            if(Private::ioctl(d->v4l_fd, VIDIOC_STREAMON, &type) < 0){
                qDebug() << "error starting streaming: " << strerror(errno);
                return;
            }
            while (d->live) {
                if(Private::ioctl(d->v4l_fd, VIDIOC_DQBUF, &bufferinfo) < 0){
                    qDebug() << "error dequeuing buffer: " << strerror(errno);
                    continue;
                }
                cv::Mat image{format.fmt.pix.height, format.fmt.pix.width, CV_8UC3};
                if(format.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG) {
                    cv::InputArray inputArray{buffer_start,  bufferinfo.bytesused};
                    image = cv::imdecode(inputArray, -1);
                } else if(format.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) {
                    cv::Mat source{format.fmt.pix.height, format.fmt.pix.width, CV_8UC2, buffer_start};
                    cv::cvtColor(source, image, CV_YUV2RGB_YVYU);
                } else {
                    qCritical() << "Unsupported image format: " << FOURCC2QS(format.fmt.pix.pixelformat);
                    return;
                }
                d->handler->handle(image);
                ++fps;
                bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                bufferinfo.memory = V4L2_MEMORY_MMAP;
                if(Private::ioctl(d->v4l_fd, VIDIOC_QBUF, &bufferinfo) < 0){
                    qDebug() << "error queuing buffer: " << strerror(errno);
                    return;
                }
            }
            Private::ioctl(d->v4l_fd, VIDIOC_DQBUF, &bufferinfo);
            if(Private::ioctl(d->v4l_fd, VIDIOC_STREAMOFF, &type) != 0) {
	      qWarning() << "error stopping live: " << strerror(errno);
	      return;
	    }
	    qDebug() << "live stopped";
	    if(-1 == munmap(buffer_start, bufferinfo.length)) {
	      qWarning() << "error unmapping memory: " << strerror(errno);
	      return;
	    }
	    qDebug() << "memory map deleted";
        }, 
        []{}, 
        [=](Thread *thread) { d->live_thread = thread; d->live = true; }
    };
}

void V4L2Imager::stopLive()
{
  if(!d->live)
    return;
  d->live = false;
  d->live_thread->wait();
  d->live_thread = nullptr;
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
    if(-1 ==Private::ioctl(v4l_fd, VIDIOC_G_FMT , &format)) {
        qWarning() << "Unable to query webcam format: " << strerror(errno);
        return {};
    } 
    return format;
}


QList< v4l2_frmsizeenum > V4L2Imager::Private::resolutions(const v4l2_format& format) const
{
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


QList< v4l2_fmtdesc > V4L2Imager::Private::formats() const
{
    v4l2_fmtdesc formats;
    QList<v4l2_fmtdesc> formats_list;
    formats.index = 0;
    formats.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while(0 ==Private::ioctl(v4l_fd, VIDIOC_ENUM_FMT, &formats)) {
        qDebug() << "found format: " << formats.index << (char*)formats.description;
        formats_list.push_back(formats);
        formats.index++;
    }
    return formats_list;
}

QList< v4l2_frmivalenum > V4L2Imager::Private::framerates(const v4l2_format& format) const
{
    v4l2_frmivalenum fps_s;
    QList<v4l2_frmivalenum> rates;
    fps_s.index = 0;
    fps_s.width = format.fmt.pix.width;
    fps_s.height = format.fmt.pix.height;
    fps_s.pixel_format = format.fmt.pix.pixelformat;
    qDebug() << "scanning for fps with pixel format " << FOURCC2QS(fps_s.pixel_format);
    while (ioctl(v4l_fd, VIDIOC_ENUM_FRAMEINTERVALS, &fps_s) >= 0) {
        qDebug() << "found fps: " << fps_s;
	rates.push_back(fps_s);
        fps_s.index++;
    }
    return rates;
}


