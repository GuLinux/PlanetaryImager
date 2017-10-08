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

#include "protocol.h"

using namespace std;

NetworkPacket::ptr NetworkProtocol::packet(const NetworkPacket::Type& name)
{
  return make_shared<NetworkPacket>(name);
}

NetworkPacket::ptr NetworkProtocol::hello(const FormatParameters &parameters)
{
  QVariantMap params {
    {"format", static_cast<int>(parameters.format) },
    {"compression", parameters.compression },
    {"force8bit", parameters.force8bit},
    {"jpegQuality", parameters.jpegQuality},
  };
  return packetHello() << params;
}

NetworkProtocol::FormatParameters NetworkProtocol::decodeHello(const NetworkPacket::ptr& packet)
{
  QVariantMap params = packet->payloadVariant().toMap();
  return {
    static_cast<Configuration::NetworkImageFormat>(params["format"].toInt()),
    params["compression"].toBool(),
    params["force8bit"].toBool(),
    params["jpegQuality"].toInt(),
  };
}


PROTOCOL_NAME_VALUE(Network, Hello);
PROTOCOL_NAME_VALUE(Network, HelloReply);
PROTOCOL_NAME_VALUE(Network, ping);
PROTOCOL_NAME_VALUE(Network, pong);


