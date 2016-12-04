/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
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

#include "driverprotocol.h"
#include <algorithm>
using namespace std;
PROTOCOL_NAME_VALUE(Driver, CameraList);
PROTOCOL_NAME_VALUE(Driver, CameraListReply);
PROTOCOL_NAME_VALUE(Driver, CamerasParameter);
PROTOCOL_NAME_VALUE(Driver, ConnectCamera);
PROTOCOL_NAME_VALUE(Driver, CameraId);
PROTOCOL_NAME_VALUE(Driver, ConnectCameraReply);
PROTOCOL_NAME_VALUE(Driver, GetCameraName);
PROTOCOL_NAME_VALUE(Driver, GetCameraNameReply);
PROTOCOL_NAME_VALUE(Driver, CameraName);
PROTOCOL_NAME_VALUE(Driver, GetProperties);
PROTOCOL_NAME_VALUE(Driver, GetPropertiesReply);
PROTOCOL_NAME_VALUE(Driver, StartLive);
PROTOCOL_NAME_VALUE(Driver, ClearROI);
PROTOCOL_NAME_VALUE(Driver, GetControls);
PROTOCOL_NAME_VALUE(Driver, GetControlsReply);


NetworkPacket::ptr DriverProtocol::sendCameraListReply(const Driver::Cameras& cameras)
{
  QVariantList v_cameras;
  transform(begin(cameras), end(cameras), back_inserter(v_cameras), [](const Driver::Camera::ptr &c){
    QVariantMap p;
    p["n"] = c->name();
    p["a"] = reinterpret_cast<qlonglong>(c.get());
    return p;
  });
  return packetCameraListReply() << NetworkPacket::Property{"cameras", v_cameras};
}



void DriverProtocol::decode(Driver::Cameras& cameras, const NetworkPacket::ptr& packet, const CameraFactory& factory)
{
  auto v_cameras = packet->property("cameras").toList();
  transform(begin(v_cameras), end(v_cameras), back_inserter(cameras), [&](const QVariant &v) { return factory(v.toMap()["n"].toString(), v.toMap()["a"].toLongLong()); });
}

NetworkPacket::ptr DriverProtocol::sendGetPropertiesReply(const Imager::Properties& properties)
{
  QVariantList l;
  QVariantList caps;
  transform(begin(properties.properties), end(properties.properties), back_inserter(l), [](const Imager::Properties::Property &p) {
    return QVariantMap {
      {"n", p.name},
      {"v", p.value},
      {"dn", p.display_name},
      {"dv", p.display_value},
    };
  });
  transform(begin(properties.capabilities), end(properties.capabilities), back_inserter(caps), [](const Imager::Capability &c) { return static_cast<int>(c); } );
  return packetGetPropertiesReply() << NetworkPacket::Property{"properties", l} << NetworkPacket::Property{"caps", caps};
}


void DriverProtocol::decode(Imager::Properties& properties, const NetworkPacket::ptr& packet)
{
  properties.capabilities.clear();
  properties.properties.clear();
  QVariantList p = packet->property("properties").toList();
  transform(begin(p), end(p), back_inserter(properties.properties), [](const QVariant &v){
    auto m = v.toMap();
    return Imager::Properties::Property{
      m["n"].toString(),
      m["v"].toString(),
      m["dn"].toString(),
      m["dv"].toString(),
    };
  });
  for(auto v: packet->property("caps").toList() )
    properties.capabilities.insert( static_cast<Imager::Capability>(v.toInt()) );
}

NetworkPacket::ptr DriverProtocol::sendGetControlsReply(const Imager::Controls& controls)
{
  QVariantList v;
  transform(begin(controls), end(controls), back_inserter(v), [](const Imager::Control &c){
    QVariantList choices;
    transform(begin(c.choices), end(c.choices), back_inserter(choices), [](const Imager::Control::Choice c){
      return QVariantMap{ {"label", c.label}, {"val", c.value} };
    });
    return QVariantMap {
      {"id",  c.id},
      {"name",  c.name},
      {"val",  c.value},
      {"def",  c.default_value},
      {"type",  c.type},
      {"min",  c.range.min},
      {"max",  c.range.max},
      {"step",  c.range.step},
      {"choices", choices},
      {"decimals", c.decimals},
      {"is_duration", c.is_duration},
      {"has_auto", c.supports_auto},
      {"is_auto", c.value_auto},
      {"ro", c.readonly},
      {"duration_unit", c.duration_unit.count()},
    };
  });
  return packetGetControlsReply() << NetworkPacket::Property{"controls", v};
}


void DriverProtocol::decode(Imager::Controls& controls, const NetworkPacket::ptr& packet)
{
  controls.clear();
  for(auto c: packet->property("controls").toList()) {
    QVariantMap ctrl = c.toMap();
    Imager::Control::Choices choices;
    for(auto c: ctrl["choices"].toList()) {
      QVariantMap choice = c.toMap();
      choices.push_back({choice["label"].toString(), choice["val"]});
    }
    
    controls.push_back( {
      ctrl["id"].toLongLong(),
      ctrl["name"].toString(),
      ctrl["type"].value<Imager::Control::Type>(),
      ctrl["val"],
      ctrl["def"],
      { ctrl["min"], ctrl["max"], ctrl["step"] },
      choices,
      static_cast<qint16>(ctrl["decimals"].toInt()),
      ctrl["is_duration"].toBool(),
      ctrl["has_auto"].toBool(),
      ctrl["is_auto"].toBool(),
      ctrl["ro"].toBool(),
      std::chrono::duration<double>{ctrl["duration_unit"].toDouble()},
    });
  }
}
