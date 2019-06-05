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

#ifndef V4L2CONTROL_H
#define V4L2CONTROL_H

#include "c++/dptr.h"
#include "drivers/imager.h"
#include <functional>
#include <QList>
#include "commons/fwd.h"

FWD_PTR(V4L2Device)

class V4L2Control
{
public:
  typedef std::shared_ptr<V4L2Control> ptr;
  typedef std::function<void(Imager::Control &)> Fix;
  
  V4L2Control(uint32_t control_id, const V4L2DevicePtr &camera, const QList<Fix> &fixes);
  ~V4L2Control();
  void set(const Imager::Control &control);
  Imager::Control control() const;
  Imager::Control update();
private:
  DPTR
};

#endif // V4L2CONTROL_H
