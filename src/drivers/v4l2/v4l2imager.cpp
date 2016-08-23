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

#include "v4l2imager_p.h"
#include "c++/stlutils.h"
#include "Qt/benchmark.h"
#include "utils.h"
using namespace std;
using namespace GuLinux;

V4L2Imager::V4L2Imager(const QString &name, int index, const ImageHandlerPtr &handler)
    : dptr(handler, "/dev/video%1"_q % index, this)
{
  d->populate_rules();
  d->open_camera();
  auto settings = this->controls();
  auto formats = find_if(begin(settings), end(settings), [](const Control &s) { return s.id == PIXEL_FORMAT_CONTROL_ID; });
  if(formats != end(settings)) {
      auto mjpeg = find_if(begin((*formats).choices), end((*formats).choices), [](const Control::Choice &c) { return c.label == "MJPG"; });
      if(mjpeg != end((*formats).choices)) {
          (*formats).value = (*mjpeg).value;
          setControl(*formats);
      };
  }
  settings = this->controls();
  auto resolutions = find_if(begin(settings), end(settings), [](const Control &s) { return s.id == RESOLUTIONS_CONTROL_ID; });
  if(resolutions != end(settings)) {
      auto v4l_resolutions = d->resolutions(d->query_format());
      auto max_resolution = max_element(begin(v4l_resolutions), end(v4l_resolutions), [](const v4l2_frmsizeenum &a, const v4l2_frmsizeenum &b){
          return a.discrete.width * a.discrete.height < b.discrete.width * b.discrete.height;
    });
    (*resolutions).value = (*max_resolution).index;
    setControl(*resolutions);
  }
  
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

QDebug operator<<(QDebug dbg, const V4L2Device::exception &e) {
  dbg.nospace().noquote() << QString{e.what()};
  return dbg.space().maybeQuote();
}

template<typename T>
void V4L2Device::ioctl(uint64_t ctl, T* data, const QString& errorLabel) const
{
    int r;
    do {
        r = ::ioctl(fd, ctl, data);
    } while (-1 == r && EINTR == errno);
    if(r == -1)
      throw V4L2Device::exception(errorLabel.isEmpty() ? "ioctl %1"_q % ctl : errorLabel);
}


template<typename T>
int V4L2Device::xioctl(uint64_t ctl, T* data, const QString& errorLabel) const
{
  try {
    this->ioctl(ctl, data, errorLabel);
    return 0;
  } catch(V4L2Device::exception &e) {
    qWarning() << e;
    return -1;
  }
}



const char* V4L2Device::exception::what() const noexcept
{
  stringstream s;
  s << "V4LDevice::exception at " << label.toStdString() << ": " << strerror(_error_code);
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


Imager::Controls V4L2Imager::controls() const
{
    Imager::Controls _settings;

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
    Control formats_setting{PIXEL_FORMAT_CONTROL_ID, "Format", 0, formats.size()-1., 1, 0, 0, Control::Combo};
    for(auto format: formats) {
      formats_setting.choices.push_back({FOURCC2QS(format.pixelformat), static_cast<double>(format.index)});
      if(format.pixelformat == current_format.fmt.pix.pixelformat)
	formats_setting.value = format.index;
    }
    _settings.push_back(formats_setting);
    auto resolutions = d->resolutions(current_format);
    Control resolutions_setting{RESOLUTIONS_CONTROL_ID, "Resolution", 0, resolutions.size()-1., 1, 0, 0, Control::Combo};
    for(auto resolution: resolutions) {
      resolutions_setting.choices.push_back({"%1x%2"_q % resolution.discrete.width % resolution.discrete.height, static_cast<double>(resolution.index)});
      if(current_format.fmt.pix.width == resolution.discrete.width && current_format.fmt.pix.height == resolution.discrete.height)
	resolutions_setting.value = resolution.index;
    }
    _settings.push_back(resolutions_setting);
    
    std::sort(begin(_settings), end(_settings), [](const Control &a, const Control &b){ return a.id < b.id; });
    _settings.erase(std::unique(begin(_settings), end(_settings), [](const Control &a, const Control &b){ return a.id == b.id; }), end(_settings));
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
    static QMap<int, Control::Type> types {
        {V4L2_CTRL_TYPE_INTEGER, Control::Number},
        {V4L2_CTRL_TYPE_BOOLEAN, Control::Bool},
        {V4L2_CTRL_TYPE_MENU, Control::Combo},
    };
    setting.unknown_type = types.count(ctrl.type) == 0;
    if(setting.unknown_type)
        return setting;
    
    v4l2_control control{ctrl.id};
    setting.valuecode = device->xioctl(VIDIOC_G_CTRL, &control, "getting control value");
    if (-1 == setting.valuecode) {
        return setting;
    }
    setting.setting = Imager::Control{ctrl.id, reinterpret_cast<char*>(ctrl.name), static_cast<double>(ctrl.minimum), static_cast<double>(ctrl.maximum), 
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

void V4L2Imager::setControl(const Control &setting)
{
  auto restart_camera = [=](function<void()> on_restart) {
    bool live_was_started = d->imager_thread.operator bool();
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
  bufferinfo.index = index;
  
  v4ldevice->ioctl(VIDIOC_QUERYBUF, &bufferinfo, "allocating buffers");

  memory = (char*) mmap(NULL, bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, v4ldevice->descriptor(), bufferinfo.m.offset);
  if(memory == MAP_FAILED){
    throw V4L2Device::exception("mapping memory");
  }
  memset(memory, 0, bufferinfo.length);
}

void V4LBuffer::queue()
{
  v4ldevice->ioctl(VIDIOC_QBUF, &bufferinfo, "queuing buffer");
}

std::shared_ptr< V4LBuffer > V4LBuffer::List::dequeue(const shared_ptr<V4L2Device> &device) const
{
    v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    device->ioctl(VIDIOC_DQBUF, &bufferinfo, "dequeuing buffer");
    auto buffer = this->at(bufferinfo.index);
    buffer->bufferinfo = bufferinfo;
    return buffer;
}


V4LBuffer::~V4LBuffer()
{
  if(-1 == munmap(memory, bufferinfo.length)) {
    qWarning() << "error unmapping memory: " << strerror(errno);
    return;
  }
  qDebug() << "memory map deleted";
}



V4L2Imager::Private::Worker::Worker(V4L2Imager::Private* d) : d{d}
{

}

bool V4L2Imager::Private::Worker::shoot(const ImageHandlerPtr& imageHandler)
{
  try {
    QBENCH(dequeue_buffer)->every(100)->ms();
    auto buffer = buffers.dequeue(d->device);
    BENCH_END(dequeue_buffer);
    QBENCH(decode_image)->every(100)->ms();
    cv::Mat image{static_cast<int>(format.fmt.pix.height), static_cast<int>(format.fmt.pix.width), CV_8UC3};
    if(format.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG) {
        cv::InputArray inputArray{buffer->memory,  buffer->bufferinfo.bytesused};
        image = cv::imdecode(inputArray, -1);
    } else if(format.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) {
        cv::Mat source{static_cast<int>(format.fmt.pix.height), static_cast<int>(format.fmt.pix.width), CV_8UC2, buffer->memory };
        cv::cvtColor(source, image, CV_YUV2RGB_YVYU);
    } else {
        qCritical() << "Unsupported image format: " << FOURCC2QS(format.fmt.pix.pixelformat);
        return false; // TODO: throw exception?
    }
    BENCH_END(decode_image);
    d->handler->handle(image);
    buffer->queue();
    return true;
  } catch(V4L2Device::exception &e) {
    qWarning() << e;
  }
}

void V4L2Imager::Private::Worker::start()
{
  format = d->query_format();
  qDebug() << "format: " << FOURCC2QS(format.fmt.pix.pixelformat) << ", " << format.fmt.pix.width << "x" << format.fmt.pix.height;
  
  v4l2_requestbuffers bufrequest;
  bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufrequest.memory = V4L2_MEMORY_MMAP;
  bufrequest.count = 4;
  d->device->ioctl(VIDIOC_REQBUFS, &bufrequest, "requesting buffers");
  
  for(int i=0; i<bufrequest.count; i++) {
    buffers.push_back( make_shared<V4LBuffer>(i, d->device));
    buffers[i]->queue();
  }
                                  
  bufferinfo_type = buffers[0]->bufferinfo.type;
  d->device->ioctl(VIDIOC_STREAMON, &bufferinfo_type, "starting streaming");
}

void V4L2Imager::Private::Worker::stop()
{
  buffers.clear();
  d->device->ioctl(VIDIOC_STREAMOFF, &bufferinfo_type, "stopping live");
  qDebug() << "live stopped";
}


void V4L2Imager::startLive()
{
  d->imager_thread = make_shared<ImagerThread>(make_shared<Private::Worker>(d.get()), this, d->handler);
  d->imager_thread->start();
}

void V4L2Imager::stopLive()
{
  d->imager_thread.reset();
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


