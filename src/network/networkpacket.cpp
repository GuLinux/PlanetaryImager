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

#include "networkpacket.h"
#include <QDataStream>
using namespace std;

DPTR_IMPL(NetworkPacket) {
  QVariantMap properties;
};

NetworkPacket::NetworkPacket() : dptr()
{
}

NetworkPacket::~NetworkPacket()
{
}

void NetworkPacket::sendTo(QDataStream &stream) const
{
  stream << d->properties;
}

void NetworkPacket::receiveFrom(QDataStream &stream)
{
  stream >> d->properties;
}

NetworkPacket * NetworkPacket::setProperty(const QString& property, const QVariant& value)
{
  d->properties[property] = value;
  return this;
}

QVariant NetworkPacket::property(const QString& name) const
{
  return d->properties[name];
}

QString NetworkPacket::name() const
{
  return property("name").toString();
}

NetworkPacket *NetworkPacket::setName(const QString& name)
{
  return setProperty("name", name);
}



QDebug operator<<(QDebug dbg, const NetworkPacket& packet)
{
  dbg << packet.d->properties;
  return dbg;
}
