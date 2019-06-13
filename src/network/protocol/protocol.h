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
#include "commons/configuration.h"
#include "network/protocol/networkpackettype.h"
#include "commons/fwd.h"

FWD_PTR(NetworkPacket)

#define ADD_PROTOCOL_NAME(name) static const NetworkPacketType name;

#define ADD_PROTOCOL_PACKET_NAME(name) ADD_PROTOCOL_NAME(name) \
static NetworkPacketPtr packet ## name() { return NetworkProtocol::packet(name); }

#define PROTOCOL_NAME__CUSTOM_VALUE(Area, name, value) const NetworkPacketType Area ## Protocol::name = value
#define PROTOCOL_NAME_VALUE(Area, name) PROTOCOL_NAME__CUSTOM_VALUE(Area, name, #Area "_" #name)



class NetworkProtocol {
public:
  static NetworkPacketPtr packet(const NetworkPacketType &name);

  ADD_PROTOCOL_PACKET_NAME(Hello)
  ADD_PROTOCOL_PACKET_NAME(HelloReply)
  ADD_PROTOCOL_PACKET_NAME(ping)
  ADD_PROTOCOL_PACKET_NAME(pong)
  struct FormatParameters {
    Configuration::NetworkImageFormat format;
    bool compression;
    bool force8bit;
    int jpegQuality;
  };
  static NetworkPacketPtr hello(const FormatParameters &parameters);
  static FormatParameters decodeHello(const NetworkPacketPtr &packet);
  
};

#endif
