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

#include "configurationprotocol.h"
#include "network/networkpacket.h"

PROTOCOL_NAME_VALUE(Configuration, List);
PROTOCOL_NAME_VALUE(Configuration, ListReply);
PROTOCOL_NAME_VALUE(Configuration, Get);
PROTOCOL_NAME_VALUE(Configuration, GetReply);
PROTOCOL_NAME_VALUE(Configuration, Set);
PROTOCOL_NAME_VALUE(Configuration, Reset);
PROTOCOL_NAME_VALUE(Configuration, signalSettingsChanged);


NetworkPacketPtr  ConfigurationProtocol::get(const QString name)
{
  return packetGet() << QVariant{name};
}

NetworkPacketPtr ConfigurationProtocol::set(const QString name, const QVariant& value)
{
  return packetSet() << QVariantMap{{"name", name}, {"value", value}};
}

NetworkPacketPtr ConfigurationProtocol::reset(const QString name)
{
  return packetReset() << QVariant{name};
}

void ConfigurationProtocol::decodeGetReply(const NetworkPacketPtr& packet, QString& name, QVariant& value)
{
  QVariantMap map = packet->payloadVariant().toMap();
  name = map["name"].toString();
  value = map["value"];
}

void ConfigurationProtocol::decodeSet(const NetworkPacketPtr& packet, QString& name, QVariant& value)
{
  QVariantMap map = packet->payloadVariant().toMap();
  name = map["name"].toString();
  value = map["value"];
}

NetworkPacketPtr ConfigurationProtocol::encodeGetReply(const QString& name, const QVariant& value)
{
  return packetGetReply() << QVariantMap {{"name", name}, {"value", value}};
}
