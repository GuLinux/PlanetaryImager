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

#ifndef NETWORKPACKET_H
#define NETWORKPACKET_H

#include "c++/dptr.h"
#include <QString>
#include <QVariant>
#include <QDebug>

class QIODevice;
class NetworkPacket
{
public:
  typedef std::shared_ptr<NetworkPacket> ptr;
  typedef QString Type;
  NetworkPacket();
  NetworkPacket(const Type &name);
  ~NetworkPacket();
  void sendTo(QIODevice *device) const;
  void receiveFrom(QIODevice *device);
  void setName(const Type &name);
  Type name() const;
  
  void setPayload(const QByteArray &payload);
  void setPayload(const QVariant &payload);
  
  QByteArray payload() const;
  QVariant payloadVariant() const;
  
  friend QDebug operator<<(QDebug dbg, const NetworkPacket &packet);
private:
  DPTR
};
NetworkPacket::ptr operator<<(const NetworkPacket::ptr &packet, const QByteArray &payload);
NetworkPacket::ptr operator<<(const NetworkPacket::ptr &packet, const QVariant &payload);

QDebug operator<<(QDebug dbg, const NetworkPacket &packet);
inline QDebug operator<<(QDebug dbg, const NetworkPacket::ptr &packet) { return dbg << *packet; }
Q_DECLARE_METATYPE(NetworkPacket::ptr)
#endif // NETWORKPACKET_H
