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

QDebug operator<<(QDebug dbg, const Imager::Setting& setting)
{
  static QMap<Imager::Setting::Type, QString> types_map {
    {Imager::Setting::Number, "Number"},
    {Imager::Setting::Combo, "Combo"},
    {Imager::Setting::Bool, "Bool"},
  };
  dbg.nospace() << "{ id:" << setting.id << ", name: " << setting.name << ", min: " << setting.min << ", max: " << setting.max << ", step: " << setting.step << ", value: " << setting.value 
  << ", type: " << types_map[setting.type] << ", choices: " << setting.choices << ", default: " << setting.defaut_value << " }";
  return dbg.space();
}

QDebug operator<<(QDebug dbg, const Imager::Setting::Choice &choice)
{
    dbg.nospace() << "{" << choice.label << "," << choice.value << "}";
    return dbg.space();
}

Imager::Setting::operator bool() const
{
    return ! name.isEmpty();
}

