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
#include "commons/utils.h"

#include "v4l2device.h"
#include "v4l2exception.h"
#include "v4l2imagingworker.h"

using namespace std;
using namespace GuLinux;

V4L2Imager::V4L2Imager(const QString &name, int index, const ImageHandlerPtr &handler)
    : dptr(handler, "/dev/video%1"_q % index, this)
{
  d->populate_control_fixes();
  d->open_camera();
  d->find_controls();
  auto settings = this->controls();
  auto formats = find_if(begin(settings), end(settings), [](const Control &s) { return s.id == PIXEL_FORMAT_CONTROL_ID; });
  if(formats != end(settings)) {
      auto mjpeg = find_if(begin((*formats).choices), end((*formats).choices), [](const Control::Choice &c) { return c.label == "MJPG"; });
      if(mjpeg != end((*formats).choices)) {
          (*formats).value = (*mjpeg).value;
          setControl(*formats);
      };
  }
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





void V4L2Imager::Private::open_camera() {
  device = make_shared<V4L2Device>(device_path);
  v4l2_capability cap;
  if(-1 == device->xioctl(VIDIOC_QUERYCAP, &cap, "query cam capabilities")) {
      return;
  }
  driver = QString::fromLocal8Bit(reinterpret_cast<char*>(cap.driver));
  bus = QString::fromLocal8Bit(reinterpret_cast<char*>(cap.bus_info));
  cameraname = QString::fromLocal8Bit(reinterpret_cast<char*>(cap.card));
}

V4L2Imager::~V4L2Imager()
{
    stopLive();
}

Imager::Properties V4L2Imager::chip() const
{
    return {};
}

QString V4L2Imager::name() const
{
    return d->cameraname;
}

void V4L2Imager::Private::find_controls()
{
  controls.clear();
  V4L2Control::ptr control;
  // TODO: should we really add controls, even if we cannot read values?
  for (int ctrlid = V4L2_CID_BASE; ctrlid < V4L2_CID_LASTP1; ctrlid++) {
    try {
      controls.push_back(control = make_shared<V4L2Control>(ctrlid, device, control_fixes));
      control->update();
    } catch(const V4L2Exception &e) {
//       if(e.type() == V4L2Exception::v4l2_error && e.code() == EINVAL)
//         break;
      qDebug() << e.what();
    }
  }

  for (int ctrlid = V4L2_CID_PRIVATE_BASE;; ctrlid++) {
    try {
      controls.push_back(control = make_shared<V4L2Control>(ctrlid, device, control_fixes));
      control->update();
    } catch(const V4L2Exception &e) {
      if(e.type() == V4L2Exception::v4l2_error && e.code() == EINVAL)
        break;
      qDebug() << e.what();
    }
  }

  int ctrlid = V4L2_CTRL_FLAG_NEXT_CTRL;
  while(true) {
    try {
      controls.push_back(control = make_shared<V4L2Control>(ctrlid, device, control_fixes));
      control->update();
    } catch(const V4L2Exception &e) {
      if(e.type() == V4L2Exception::v4l2_error && e.code() == EINVAL)
        break;
      qDebug() << e.what();
    }
    ctrlid = static_cast<uint32_t>(control->control().id) | V4L2_CTRL_FLAG_NEXT_CTRL;
  }
}


Imager::Controls V4L2Imager::controls() const
{
    Imager::Controls _settings;
    transform(begin(d->controls), end(d->controls), back_inserter(_settings), [](const V4L2Control::ptr &c) { return c->control(); } );
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



void V4L2Imager::setControl(const Control &setting)
{
  // TODO: huge refactoring needed here...
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
      try {
        d->device->ioctl(VIDIOC_S_FMT , &current_format, "setting webcam format");
      }
      catch(const V4L2Exception &e) {
        qWarning() << e.what();
      }
      auto format = d->query_format();
      for(auto avail_format: d->formats()) {
        if(avail_format.pixelformat == format.fmt.pix.pixelformat) {
          auto changed_setting = setting;
          changed_setting.value = avail_format.index;
          emit changed(changed_setting);
          return;
        }
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
      try {
        d->device->ioctl(VIDIOC_S_FMT , &current_format, "setting resolution");
      } catch(const V4L2Exception &e) {
        qWarning() << e.what();
      }
      auto format = d->query_format();
      for(auto resolution: d->resolutions(format)) {
        if(resolution.discrete.width == format.fmt.pix.width && resolution.discrete.height == format.fmt.pix.height) {
          auto changed_setting = setting;
          changed_setting.value = resolution.index;
          emit changed(changed_setting);
          return;
        }
      }
    });
    return;
  }
  auto control = find_if(begin(d->controls), end(d->controls), [=](const V4L2Control::ptr &c) { return setting.id == c->control().id; });
  if(control != end(d->controls)) {
    d->imager_thread->push_job([=]{
      try {
        (*control)->set(setting);
      } catch(const V4L2Exception &e) {
        qWarning() << e.what();
      }
      emit changed((*control)->update());
    });
  }
}

void V4L2Imager::startLive()
{
  d->imager_thread = make_shared<ImagerThread>(make_shared<V4L2ImagingWorker>(d->device, d->query_format()), this, d->handler);
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
        qDebug() << "Found resolution: " << frmsize.discrete.width << "x" << frmsize.discrete.height;
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


