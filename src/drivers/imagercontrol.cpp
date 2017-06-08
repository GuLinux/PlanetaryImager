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
#include "Qt/strings.h"
#include <QHash>
#include <functional>

using namespace std;
using namespace std::placeholders;

QDebug operator<<(QDebug dbg, const Imager::Control::Range& r)
{
  dbg.nospace() << "min=" << r.min << ", max=" << r.max << ", step=" << r.step;
  return dbg;
}

QDebug operator<<(QDebug dbg, const Imager::Control& setting)
{
  static QMap<Imager::Control::Type, QString> types_map {
    {Imager::Control::Number, "Number"},
    {Imager::Control::Combo, "Combo"},
    {Imager::Control::Bool, "Bool"},
  };
  dbg.nospace() << "{ id:" << setting.id << ", name: " << setting.name << ", range: " << setting.range << ", value: " << setting.value << ", auto: " << setting.value_auto
  << ", type: " << types_map[setting.type] << ", choices: " << setting.choices << ", default: " << setting.default_value << " }";
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


bool Imager::Control::same_value(const Imager::Control& other) const
{
    bool autoChanged = false;

    if(supports_auto && (value_auto || other.value_auto) ) {
      qDebug() << "comparing auto values only for control " << name << "[" << id << "]";

      autoChanged = (other.value_auto != value_auto);
    }

    const bool onOffChanged = (value_onOff != other.value_onOff);

    return !autoChanged && !onOffChanged && value == other.value;
}

QVariantMap Imager::Control::asMap() const
{
    QVariantMap data;
    data["name"] = name;
    data["value"] = value;
    static QMap<Imager::Control::Type, QString> types {
      {Imager::Control::Number, "number"},
      {Imager::Control::Combo, "combo"},
      {Imager::Control::Bool, "bool"}
    };
    data["type"] = types[type];
    data["id"] = id;
    if(type == Imager::Control::Combo) {
      QVariantMap choices;
      for(auto choice: this->choices)
        choices[choice.label] = choice.value;
        data["choices"] = choices;
    }
    if(type == Imager::Control::Number && is_duration) {
      data["type"] = "duration";
      QList<QPair<QString, double>> units { {"seconds", 1}, {"milliseconds", 1000}, {"microseconds", 1000000} };
      for(auto unit: units) {
        data["value_%1"_q % unit.first] = value.toDouble() * duration_unit.count() * unit.second;
      }
    }
    data["auto"] = value_auto;
    return data;
}


void Imager::Control::import(const QVariantMap& data, bool full_import)
{
  value = data["value"];
  value_auto = data["auto"].toBool();
  if(!full_import)
    return;
  name = data["name"].toString();
  id = data["id"].toLongLong();
  is_duration = false;
  QHash<QString, function<void()>> types_map {
    {"number", [&]{ type = Number; } },
    {"combo", [&]{ type = Combo; } },
    {"bool", [&]{ type = Bool; } },
    {"duration", [&]{ type = Number; is_duration = true; } },
  };
  types_map[data["type"].toString()]();
}
