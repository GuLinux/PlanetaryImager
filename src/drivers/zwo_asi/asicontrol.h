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

#ifndef ASICONTROL_H
#define ASICONTROL_H

#include "ASICamera2.h"
#include "drivers/imager.h"
#include "commons/fwd.h"


FWD_PTR(ASIControl)

struct ASIControl {
  typedef std::vector<ASIControlPtr> vector;
  int index;
  int camera_id;
  ASIControl(int index, int camera_id);
  ASI_CONTROL_CAPS caps;
  long value;
  bool is_auto;

  Imager::Control control() const;
  operator Imager::Control() const;
  ASIControl &reload();
  ASIControl &set(qlonglong new_value, bool is_auto);
};


#endif // ASICONTROL_H
