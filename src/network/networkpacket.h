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
  typedef QString NameType;;
  typedef QString KeyType;
  struct Property {
    KeyType key;
    QVariant value;
  };
  NetworkPacket();
  NetworkPacket(const NameType &name);
  ~NetworkPacket();
  void sendTo(QIODevice *device) const;
  void receiveFrom(QIODevice *device);
  NetworkPacket *setName(const NameType &name);
  NameType name() const;
  NetworkPacket *setProperty(const KeyType &property, const QVariant &value);
  QVariant property(const KeyType &name) const;
  friend QDebug operator<<(QDebug dbg, const NetworkPacket &packet);
private:
  DPTR
};
NetworkPacket::ptr operator<<(NetworkPacket::ptr packet, const NetworkPacket::Property &property);

QDebug operator<<(QDebug dbg, const NetworkPacket &packet);
inline QDebug operator<<(QDebug dbg, const NetworkPacket::ptr &packet) { return dbg << *packet; }
Q_DECLARE_METATYPE(NetworkPacket::ptr)
#endif // NETWORKPACKET_H
