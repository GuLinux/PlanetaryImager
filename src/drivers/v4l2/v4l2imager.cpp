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

#include "c++/stlutils.h"
#include "Qt/benchmark.h"
#include "commons/utils.h"

#include "v4l2imager.h"
#include "v4l2device.h"
#include "v4l2exception.h"
#include "v4l2imagingworker.h"
#include "v4l2formats.h"
#include "v4l2control.h"
#include "v4l2imager_rules.h"
#include <QThread>

using namespace std;
using namespace GuLinux;


#define PIXEL_FORMAT_CONTROL_ID -10
#define RESOLUTIONS_CONTROL_ID -9
#define FPS_CONTROL_ID -8


DPTR_IMPL(V4L2Imager)
{
    class Worker;
    const QString device_path;
    Drivers::V4L2::ControlFixes control_fixes;
    V4L2Imager *q;
    
    V4L2Device::ptr device;
    
    V4L2Formats::ptr v4l2formats;
    QList<V4L2Formats::Resolution::ptr> resolutions;

    void open_camera();
    QList<V4L2Control::ptr> controls;
    QString driver, bus, cameraname;
    QString dev_name;
    void find_controls();
};


V4L2Imager::V4L2Imager(const QString &name, int index, const ImageHandler::ptr &handler)
    : Imager{handler}, dptr("/dev/video%1"_q % index, Drivers::V4L2::controlFixes(this), this)
{
  d->open_camera();
  d->v4l2formats = make_shared<V4L2Formats>(d->device);
  for(auto format: d->v4l2formats->formats())
    for(auto resolution: format->resolutions())
      d->resolutions.push_back(resolution);
    // TODO: restore mjpeg as first if found
  d->find_controls();
  auto settings = this->controls();
}

void V4L2Imager::Private::open_camera() {
  LOG_F_SCOPE
  device = make_shared<V4L2Device>(device_path);
  v4l2_capability cap;
  device->ioctl(VIDIOC_QUERYCAP, &cap, "query cam capabilities");
  driver = QString::fromLocal8Bit(reinterpret_cast<char*>(cap.driver));
  bus = QString::fromLocal8Bit(reinterpret_cast<char*>(cap.bus_info));
  cameraname = QString::fromLocal8Bit(reinterpret_cast<char*>(cap.card));
}

V4L2Imager::~V4L2Imager()
{
}

Imager::Properties V4L2Imager::properties() const
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
    auto current_resolution = d->v4l2formats->current_resolution();
    
    Control resolutions_setting{RESOLUTIONS_CONTROL_ID, "Resolution", 0, d->resolutions.size()-1., 1, 0, 0, Control::Combo};
    int index = 0;
    for(auto resolution: d->resolutions) {
      resolutions_setting.choices.push_back({"%1 %2x%3"_q % resolution->format().name() % resolution->size().width() % resolution->size().height(), static_cast<double>(index)});
      if(resolution == current_resolution)
        resolutions_setting.value = static_cast<double>(index);
      index++;
    }
    _settings.push_back(resolutions_setting);
    
    std::sort(begin(_settings), end(_settings), [](const Control &a, const Control &b){ return a.id < b.id; });
    _settings.erase(std::unique(begin(_settings), end(_settings), [](const Control &a, const Control &b){ return a.id == b.id; }), end(_settings));
    return _settings;
}



void V4L2Imager::setControl(const Control &setting)
{
  if(setting.id == RESOLUTIONS_CONTROL_ID) {
    restart([=]{
      try {
        d->resolutions[setting.value]->set();
      } catch(const V4L2Exception &e) {
        qWarning() << "Unable to set resolution: " << e.what();
      }
      return make_shared<V4L2ImagingWorker>(d->device, d->v4l2formats->current_v4l2_format());
    });

    auto current = d->v4l2formats->current_resolution();
    Control new_value = setting;
    new_value.value = find_if( d->resolutions.begin(), d->resolutions.end(), [&](const V4L2Formats::Resolution::ptr &r){ return *r == *current; } ) - d->resolutions.begin();
    emit changed(new_value);
    return;
  }
  auto control = find_if(begin(d->controls), end(d->controls), [=](const V4L2Control::ptr &c) { return setting.id == c->control().id; });
  if(control != end(d->controls)) {
    push_job_on_thread([=]{
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
  restart([=]{ return make_shared<V4L2ImagingWorker>(d->device, d->v4l2formats->current_v4l2_format()); });
}
