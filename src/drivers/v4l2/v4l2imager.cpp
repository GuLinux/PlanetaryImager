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

using namespace std;
using namespace GuLinux;


#define PIXEL_FORMAT_CONTROL_ID -10
#define RESOLUTIONS_CONTROL_ID -9
#define FPS_CONTROL_ID -8


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

void V4L2Imager::Private::populate_control_fixes()
{
 // Exposure to combo in Microsoft Lifecam 3000
  control_fixes.push_back( [=](Control &s){
   if(s.id != V4L2_CID_EXPOSURE_ABSOLUTE || (cameraname != "Microsoft\u00AE LifeCam HD-3000" && cameraname != "Microsoft\u00AE LifeCam HD-5000"))
      return;
   s.type = Control::Combo;
   s.choices = {{"5", 5}, {"9", 9}, {"10", 10}, {"19", 19}, {"20", 20}, {"39", 39}, {"78", 78}, {"156", 156}, {"312", 312}, {"625", 625}, {"1250", 1250}, {"2500", 2500}, {"5000", 5000}, {"10000", 10000}};
  });
}



V4L2Imager::V4L2Imager(const QString &name, int index, const ImageHandler::ptr &handler)
    : dptr(handler, "/dev/video%1"_q % index, this)
{
  d->populate_control_fixes();
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
    stopLive();
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
    stopLive();
    d->open_camera();
    d->resolutions[setting.value]->set();
    startLive();
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
  d->imager_thread = make_shared<ImagerThread>(make_shared<V4L2ImagingWorker>(d->device, d->v4l2formats->current_v4l2_format()), this, d->handler);
  d->imager_thread->start();
}

void V4L2Imager::stopLive()
{
  d->imager_thread.reset();
}




