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

#ifndef CONFIGURATIONPROTOCOL_H
#define CONFIGURATIONPROTOCOL_H

#include "network/protocol/protocol.h"
#include "commons/fwd.h"
FWD_PTR(NetworkPacket)

class ConfigurationProtocol : public NetworkProtocol
{
public:
  ADD_PROTOCOL_PACKET_NAME(List)
  ADD_PROTOCOL_PACKET_NAME(ListReply)
  ADD_PROTOCOL_PACKET_NAME(Get)
  ADD_PROTOCOL_PACKET_NAME(GetReply)
  ADD_PROTOCOL_PACKET_NAME(Set)
  ADD_PROTOCOL_PACKET_NAME(Reset)
  ADD_PROTOCOL_PACKET_NAME(signalSettingsChanged)
  
  static NetworkPacketPtr get(const QString name);
  static NetworkPacketPtr set(const QString name, const QVariant &value);
  static NetworkPacketPtr reset(const QString name);
  
  static void decodeGetReply(const NetworkPacketPtr &packet, QString &name, QVariant &value);
  static NetworkPacketPtr encodeGetReply(const QString &name, const QVariant &value);
  
  static void decodeSet(const NetworkPacketPtr &packet, QString &name, QVariant &value);
};

#endif // CONFIGURATIONPROTOCOL_H
