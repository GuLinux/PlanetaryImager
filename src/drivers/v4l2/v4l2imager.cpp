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
#include "c++/stlutils.h"
#include "utils.h"
using namespace std;
using namespace GuLinux;

V4L2Imager::Private::Private(const ImageHandlerPtr& handler, const QString& device_path, V4L2Imager* q) : handler{handler}, device_path{device_path}, q(q)
{
}

V4L2Imager::V4L2Imager(const QString &name, int index, const ImageHandlerPtr &handler)
    : dptr(handler, "/dev/video%1"_q % index, this)
{
  d->populate_rules();
  d->open_camera();
  
  d->adjust_framerate(d->query_format());
}



V4L2Device::V4L2Device(const QString& path) : _path{path}
{
    fd = ::open(path.toLatin1(), O_RDWR, O_NONBLOCK, 0);
    if (-1 == fd) {
      throw V4L2Device::exception("opening device '%1'"_q % path);
    }
}

V4L2Device::~V4L2Device()
{
  if(-1 != fd)
    ::close(fd);
}

template<typename T>
void V4L2Device::ioctl(uint64_t ctl, T* data, const QString& errorLabel) const
{
    int r;
    do {
        r = ::ioctl(fd, ctl, data);
    } while (-1 == r && EINTR == errno);
    if(r == -1)
      throw V4L2Device::exception(errorLabel.isEmpty() ? "on ioctl %1"_q % ctl : errorLabel);
}


template<typename T>
int V4L2Device::xioctl(uint64_t ctl, T* data, const QString& errorLabel) const
{
  try {
    this->ioctl(ctl, data, errorLabel);
    return 0;
  } catch(V4L2Device::exception &e) {
    qWarning() << QString{e.what()};
    return -1;
  }
}



const char* V4L2Device::exception::what() const noexcept
{
  stringstream s;
  s << label.toStdString() << ": " << strerror(_error_code);
  return s.str().c_str();
}




void V4L2Imager::Private::open_camera() {
  device = make_shared<V4L2Device>(device_path);
  v4l2_capability cap;
  if(-1 == device->xioctl(VIDIOC_QUERYCAP, &cap, "query cam capabilities")) {
      return;
  }
  driver = {(char*) cap.driver};
  bus = {(char*) cap.bus_info};
  cameraname = {(char*) cap.card};
}

V4L2Imager::~V4L2Imager()
{
    stopLive();
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
    Setting formats_setting{PIXEL_FORMAT_CONTROL_ID, "Format", 0, formats.size()-1., 1, 0, 0, Setting::Combo};
    for(auto format: formats) {
      formats_setting.choices.push_back({FOURCC2QS(format.pixelformat), static_cast<double>(format.index)});
      if(format.pixelformat == current_format.fmt.pix.pixelformat)
	formats_setting.value = format.index;
    }
    _settings.push_back(formats_setting);
    auto resolutions = d->resolutions(current_format);
    Setting resolutions_setting{RESOLUTIONS_CONTROL_ID, "Resolution", 0, resolutions.size()-1., 1, 0, 0, Setting::Combo};
    for(auto resolution: resolutions) {
      resolutions_setting.choices.push_back({"%1x%2"_q % resolution.discrete.width % resolution.discrete.height, static_cast<double>(resolution.index)});
      if(current_format.fmt.pix.width == resolution.discrete.width && current_format.fmt.pix.height == resolution.discrete.height)
	resolutions_setting.value = resolution.index;
    }
    _settings.push_back(resolutions_setting);
    
    std::sort(begin(_settings), end(_settings), [](const Setting &a, const Setting &b){ return a.id < b.id; });
    _settings.erase(std::unique(begin(_settings), end(_settings), [](const Setting &a, const Setting &b){ return a.id == b.id; }), end(_settings));
    return _settings;
}


V4L2Imager::Private::V4lSetting V4L2Imager::Private::setting(uint32_t id)
{
    V4lSetting setting;
    v4l2_queryctrl ctrl{id};
    setting.querycode =device->xioctl(VIDIOC_QUERYCTRL, &ctrl);
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
    setting.valuecode = device->xioctl(VIDIOC_G_CTRL, &control, "getting control value");
    if (-1 == setting.valuecode) {
        return setting;
    }
    setting.setting = Imager::Setting{ctrl.id, reinterpret_cast<char*>(ctrl.name), static_cast<double>(ctrl.minimum), static_cast<double>(ctrl.maximum), 
      static_cast<double>(ctrl.step), static_cast<double>(control.value), static_cast<double>(ctrl.default_value)};
    setting.setting.type = types[ctrl.type];
    if(ctrl.type == V4L2_CTRL_TYPE_MENU) {
        v4l2_querymenu menu{ctrl.id};
        for(menu.index = ctrl.minimum; menu.index <= ctrl.maximum; menu.index++) {
            QString value;
            if (0 == device->xioctl (VIDIOC_QUERYMENU, &menu)) {
                value = {(char*)menu.name};
            }
            setting.setting.choices.push_back({value, static_cast<double>(menu.index)});
        }
    }
    qDebug() << "setting: " << setting.setting << "(exposure: " << V4L2_CID_EXPOSURE_ABSOLUTE << ")";
    for(auto rule: setting_rules)
        rule(setting.setting);
    return setting;
}

void V4L2Imager::setSetting(const Setting &setting)
{
  auto restart_camera = [=](function<void()> on_restart) {
    auto live_was_started = d->live;
    stopLive();
    d->device.reset();
    d->open_camera();
    on_restart();
    d->adjust_framerate(d->query_format());
    if(live_was_started)
      startLive();
  };
  if(setting.id == PIXEL_FORMAT_CONTROL_ID) {
    restart_camera([=]{
      auto setting_format = d->formats()[setting.value];
      auto current_format = d->query_format();
      qDebug() << "setting format: old=" << FOURCC2QS(current_format.fmt.pix.pixelformat) << ", new=" << FOURCC2QS(setting_format.pixelformat);
      current_format.fmt.pix.pixelformat = setting_format.pixelformat;
      if(-1 == d->device->xioctl(VIDIOC_S_FMT , &current_format, "setting webcam format")) {
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
      if(-1 == d->device->xioctl(VIDIOC_S_FMT , &current_format, "setting resolution")) {
	  return;
      }
    });
    return;
  }
  
  v4l2_control control{static_cast<uint32_t>(setting.id), static_cast<int>(setting.value)};
  if (-1 == d->device->xioctl (VIDIOC_S_CTRL, &control, "setting control %1-%2 value to %3"_q % setting.id % setting.name % setting.value)) {
  }
}


V4LBuffer::V4LBuffer(int index, const shared_ptr<V4L2Device> &v4ldevice) : v4ldevice {v4ldevice}
{
  memset(&bufferinfo, 0, sizeof(v4l2_buffer));
  bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufferinfo.memory = V4L2_MEMORY_MMAP;
  bufferinfo.index = 0;
  
  if(v4ldevice->xioctl(VIDIOC_QUERYBUF, &bufferinfo, "allocating buffers") < 0){
    return;
  }

  memory = (char*) mmap(NULL, bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, v4ldevice->descriptor(), bufferinfo.m.offset);
  if(memory == MAP_FAILED){
    qWarning() << "error memmapping buffer: " << strerror(errno);
    return;
  }
  memset(memory, 0, bufferinfo.length);
}

void V4LBuffer::dequeue()
{
  if(v4ldevice->xioctl(VIDIOC_DQBUF, &bufferinfo, "dequeuing buffer") < 0){
      return;
  }
}

void V4LBuffer::queue()
{
  if(v4ldevice->xioctl(VIDIOC_QBUF, &bufferinfo, "queuing buffer") < 0){
      return;
  }
}


V4LBuffer::~V4LBuffer()
{
  if(-1 == munmap(memory, bufferinfo.length)) {
    qWarning() << "error unmapping memory: " << strerror(errno);
    return;
  }
  qDebug() << "memory map deleted";
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
            bufrequest.count = 4;
            if(d->device->xioctl(VIDIOC_REQBUFS, &bufrequest, "requesting buffers") < 0) {
                return;
            }
            
            vector<shared_ptr<V4LBuffer>> buffers;
	    for(int i=0; i<bufrequest.count; i++)
	      buffers.push_back( make_shared<V4LBuffer>(i, d->device));
            
	    int buffer_index = 0;
                     

	    buffers[buffer_index]->queue();
	    
            int type = buffers[buffer_index]->bufferinfo.type;
            if(d->device->xioctl(VIDIOC_STREAMON, &type, "starting streaming") < 0){
                return;
            }
            while (d->live) {
	      auto current_index = buffer_index % bufrequest.count;
	      auto next_index = (buffer_index+1) % bufrequest.count;
		benchmark_start(dequeue_frame)
		buffers[next_index]->queue();
		buffers[current_index]->dequeue();
		benchmark_end(dequeue_frame)
		
		benchmark_start(copy_data)
                QByteArray buffer_data(buffers[current_index]->memory, buffers[current_index]->bufferinfo.bytesused);
		benchmark_end(copy_data)
		benchmark_start(converting_image)
		cv::Mat image{static_cast<int>(format.fmt.pix.height), static_cast<int>(format.fmt.pix.width), CV_8UC3};
		if(format.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG) {
		    cv::InputArray inputArray{buffer_data.data(),  buffer_data.size()};
		    image = cv::imdecode(inputArray, -1);
		} else if(format.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) {
		    cv::Mat source{static_cast<int>(format.fmt.pix.height), static_cast<int>(format.fmt.pix.width), CV_8UC2, buffer_data.data()};
		    cv::cvtColor(source, image, CV_YUV2RGB_YVYU);
		} else {
		    qCritical() << "Unsupported image format: " << FOURCC2QS(format.fmt.pix.pixelformat);
		    return;
		}
		benchmark_end(converting_image)
		benchmark_scope(handle_image)
		d->handler->handle(image);
                ++fps;
		buffer_index++;
            }
            if(d->device->xioctl(VIDIOC_STREAMOFF, &type, "stopping live") != 0) {
	      qWarning() << "error stopping live: " << strerror(errno);
	      return;
	    }
	    qDebug() << "live stopped";
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



v4l2_format V4L2Imager::Private::query_format() const
{
    v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(-1 ==device->xioctl(VIDIOC_G_FMT , &format, "querying webcam format")) {
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
    while (device->xioctl(VIDIOC_ENUM_FRAMESIZES, &frmsize, "querying resolutions") >= 0) {
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
    while(0 ==device->xioctl(VIDIOC_ENUM_FMT, &formats)) {
        qDebug() << "found format: " << formats.index << (char*)formats.description;
        formats_list.push_back(formats);
        formats.index++;
    }
    return formats_list;
}

void V4L2Imager::Private::adjust_framerate(const v4l2_format& format) const
{
    v4l2_frmivalenum fps_s;
    QList<v4l2_frmivalenum> rates;
    fps_s.index = 0;
    fps_s.width = format.fmt.pix.width;
    fps_s.height = format.fmt.pix.height;
    fps_s.pixel_format = format.fmt.pix.pixelformat;
    qDebug() << "scanning for fps with pixel format " << FOURCC2QS(fps_s.pixel_format);
    while (device->xioctl(VIDIOC_ENUM_FRAMEINTERVALS, &fps_s) >= 0) {
        qDebug() << "found fps: " << fps_s;
	rates.push_back(fps_s);
        fps_s.index++;
    }
    auto ratio = [=](const v4l2_frmivalenum &a) { return static_cast<double>(a.discrete.numerator)/static_cast<double>(a.discrete.denominator); };
    sort(begin(rates), end(rates), [&](const v4l2_frmivalenum &a, const v4l2_frmivalenum &b){ return ratio(a) < ratio(b);} );
    v4l2_streamparm streamparam;
    streamparam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(0 != device->xioctl(VIDIOC_G_PARM, &streamparam, "getting stream parameters")) {
      return;
    }
    qDebug() << "current frame rate: " << streamparam.parm.capture.timeperframe;
    streamparam.parm.capture.timeperframe = rates[0].discrete;
    if(0 != device->xioctl(VIDIOC_S_PARM, &streamparam, "setting stream parameters")) {
      return;
    }
    qDebug() << "current frame rate: " << streamparam.parm.capture.timeperframe;
}

