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

#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H

#include <QString>
#include "network/networkdispatcher.h"
#include "network/networkpacket.h"

class NetworkProtocol {
public:
  static NetworkPacket::ptr packet(const NetworkPacket::NameType &name);
};

#define ADD_PROTOCOL_NAME(name) static const NetworkPacket::NameType name;

#define ADD_PROTOCOL_PACKET_NAME(name) ADD_PROTOCOL_NAME(name) \
static NetworkPacket::ptr packet ## name() { return NetworkProtocol::packet(name); }

#define ADD_PROTOCOL_PROPERTY(name) ADD_PROTOCOL_NAME(name) \
static NetworkPacket::Property property ## name(const QVariant &value) { return {name, value}; }

#define PROTOCOL_NAME__CUSTOM_VALUE(Area, name, value) const NetworkPacket::NameType Area ## Protocol::name = value
#define PROTOCOL_NAME_VALUE(Area, name) PROTOCOL_NAME__CUSTOM_VALUE(Area, name, #Area "_" #name)
#endif
