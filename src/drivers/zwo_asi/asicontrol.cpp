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

#include "asicontrol.h"
#include "zwoexception.h"
#include "c++/stringbuilder.h"
#include <set>

using namespace std;
using namespace GuLinux;

ASIControl::ASIControl(int index, int camera_id) : index{index}, camera_id{camera_id}
{
  ASI_CHECK << ASIGetControlCaps(camera_id, index, &caps) << "Get control caps";
  reload();
}

Imager::Setting ASIControl::setting() const
{
  return *this;
}

ASIControl &ASIControl::reload()
{
  ASI_BOOL is_auto;
  ASI_CHECK << ASIGetControlValue(camera_id, caps.ControlType, &value, &is_auto)
            << (stringbuilder() << "Get control value: " << caps.Name);
  qDebug() << "Got raw control value for ControlType " << caps.ControlType << ", index " << index << ": value=" << value << ", is_auto=" << is_auto;
  this->is_auto = static_cast<bool>(is_auto);
  return *this;
}

ASIControl &ASIControl::set(double new_value, bool is_auto)
{
  long new_value_l = static_cast<long>(new_value);
  if(caps.ControlType == ASI_TEMPERATURE)
    new_value_l = static_cast<long>(new_value * 10.);
  qDebug() << "Setting control " << caps.ControlType << " to value " << new_value_l << ", auto=" << is_auto;
  ASI_CHECK << ASISetControlValue(camera_id, caps.ControlType, new_value_l, static_cast<ASI_BOOL>(is_auto))
            << (stringbuilder() << "Set new control value: " << caps.Name << " to " << new_value << " (auto: " << is_auto << ")");
  reload();
  return *this;
}

ASIControl::operator Imager::Setting() const
{
  Imager::Setting setting = {
    static_cast<int64_t>(caps.ControlType),
    caps.Description,
    static_cast<double>(caps.MinValue),
    static_cast<double>(caps.MaxValue),
    1.0,
    static_cast<double>(value),
    static_cast<double>(caps.DefaultValue),
  };
  setting.decimals = 0;

  static std::set<ASI_CONTROL_TYPE> boolean_caps {ASI_HIGH_SPEED_MODE, ASI_HARDWARE_BIN};
  if(boolean_caps.count(caps.ControlType))
    setting.type = Imager::Setting::Bool;

  if(caps.ControlType == ASI_EXPOSURE) {
    setting.is_duration = true;
    setting.duration_unit = 1us;
  }
  if(caps.ControlType == ASI_TEMPERATURE) {
    qDebug() << "Converting temperature from " << value;
    setting.decimals = 1;
    setting.value = static_cast<double>(value) / 10.;
  }
  setting.readonly = !caps.IsWritable;
  setting.value_auto = caps.IsAutoSupported && is_auto;
  setting.supports_auto = caps.IsAutoSupported;
  return setting;
}
