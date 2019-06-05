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

#include "v4l2control.h"
#include <linux/videodev2.h>
#include <QDebug>
#include <QHash>
#include "v4l2exception.h"
#include "c++/stringbuilder.h"
#include "Qt/qt_strings_helper.h"
#include "v4l2device.h"

using namespace std;
using namespace GuLinux;
DPTR_IMPL(V4L2Control) {
  const V4L2DevicePtr camera;
  Imager::Control control;
  void read();
};

V4L2Control::V4L2Control(uint32_t control_id, const V4L2DevicePtr& camera, const QList<Fix> &fixes) : dptr(camera)
{ 
  v4l2_queryctrl v4l2control{control_id};
  camera->ioctl(VIDIOC_QUERYCTRL, &v4l2control);
  d->control.name = reinterpret_cast<const char*>(v4l2control.name);
  d->control.readonly = (v4l2control.flags & V4L2_CTRL_FLAG_READ_ONLY);
  
  qDebug() << "Found v4l2 control: id=" << control_id << ", name=" << d->control.name << ", flags=" << v4l2control.flags << ", type=" << v4l2control.type << ", range=" << v4l2control.minimum << "-" << v4l2control.maximum << ", step=" << v4l2control.step << ", default value=" << v4l2control.default_value << ", readonly: " << d->control.readonly;
  
  d->control.id = v4l2control.id;
  d->control.set_range(v4l2control.minimum, v4l2control.maximum, v4l2control.step);
  d->control.set_default_value(v4l2control.default_value);
    
  if(v4l2control.flags & V4L2_CTRL_FLAG_DISABLED)
      throw V4L2Exception{ V4L2Exception::control_disabled, stringbuilder() << "Control " << v4l2control.name << " disabled", "V4L2Control()"};
  
  static QHash<int, Imager::Control::Type> types {
        {V4L2_CTRL_TYPE_INTEGER, Imager::Control::Number},
        {V4L2_CTRL_TYPE_INTEGER64, Imager::Control::Number},
        {V4L2_CTRL_TYPE_BOOLEAN, Imager::Control::Bool},
        {V4L2_CTRL_TYPE_MENU, Imager::Control::Combo},
    };
  bool unknown_type = types.count(v4l2control.type) == 0;
    if(unknown_type)
      throw V4L2Exception{ V4L2Exception::control_type_unknown, stringbuilder() << "Unknown control type: " << v4l2control.type, "V4L2Control()"};
    d->control.type = types[v4l2control.type];
    
    if(v4l2control.type == V4L2_CTRL_TYPE_MENU) {
        v4l2_querymenu menu{v4l2control.id};
        for(menu.index = v4l2control.minimum; menu.index <= v4l2control.maximum; menu.index++) {
            QString value;
            if (0 == camera->xioctl (VIDIOC_QUERYMENU, &menu)) {
                value = QString::fromLocal8Bit(reinterpret_cast<const char*>(menu.name));
            }
            d->control.add_choice(value, menu.index);
        }
    }
  for(auto fix: fixes)
    fix(d->control);
}

V4L2Control::~V4L2Control()
{
}


Imager::Control V4L2Control::control() const
{
  return d->control;
}

void V4L2Control::set(const Imager::Control& control)
{
  v4l2_control ctrl{static_cast<uint32_t>(control.id), control.get_value<int>()};
  d->camera->ioctl (VIDIOC_S_CTRL, &ctrl, "setting control %1-%2 value to %3"_q % control.id % control.name % control.value.toInt());
}

void V4L2Control::Private::read()
{
  v4l2_control control;
  control.id = static_cast<uint32_t>(this->control.id);
  camera->ioctl(VIDIOC_G_CTRL, &control, "getting control value for control %1 (id=%2)"_q % this->control.name % control.id );
  this->control.set_value(control.value);
}

Imager::Control V4L2Control::update()
{
  d->read();
  return d->control;
}

