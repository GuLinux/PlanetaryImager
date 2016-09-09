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
#include "imager.h"

QDebug operator<<(QDebug dbg, const Imager::Chip& chip)
{
  dbg.nospace() << "{ size: " << chip.width << "x" << chip.height << ", pixels size: " << chip.pixelwidth << "x" << chip.pixelheight <<
    ", image size: " << chip.xres << "x" << chip.yres << "@" << chip.bpp << "bpp }";
  return dbg.space();
}

QDebug operator<<(QDebug dbg, const Imager::Control& setting)
{
  static QMap<Imager::Control::Type, QString> types_map {
    {Imager::Control::Number, "Number"},
    {Imager::Control::Combo, "Combo"},
    {Imager::Control::Bool, "Bool"},
  };
  dbg.nospace() << "{ id:" << setting.id << ", name: " << setting.name << ", min: " << setting.min << ", max: " << setting.max << ", step: " << setting.step << ", value: " << setting.value 
  << ", type: " << types_map[setting.type] << ", choices: " << setting.choices << ", default: " << setting.defaut_value << " }";
  return dbg.space();
}

QDebug operator<<(QDebug dbg, const Imager::Control::Choice &choice)
{
    dbg.nospace() << "{" << choice.label << "," << choice.value << "}";
    return dbg.space();
}

bool Imager::Control::valid() const
{
    return ! name.isEmpty();
}

void Imager::destroy()
{
  stopLive();
  emit disconnected();
  this->deleteLater();
}


bool Imager::Control::same_value(const Imager::Control& other) const
{
    if(supports_auto && (value_auto || other.value_auto) ) {
        return other.value_auto == value_auto;
    }
    return value == other.value;
}
