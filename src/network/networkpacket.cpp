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
#include <QIODevice>
#include <QDataStream>
#include <QJsonDocument>
#include "Qt/strings.h"
#include <QCoreApplication>
#include <QBuffer>

using namespace std;

DPTR_IMPL(NetworkPacket) {
  Type name;
  QByteArray payload;
};

NetworkPacket::NetworkPacket() : dptr()
{
}

NetworkPacket::NetworkPacket(const Type& name) : NetworkPacket()
{
  setName(name);
}


NetworkPacket::~NetworkPacket()
{
}



void NetworkPacket::sendTo(QIODevice *device) const
{
  QDataStream s(device);
  s << d->name << d->payload.size();
  qint64 wrote = device->write(d->payload);
  if(wrote == -1) {
    qWarning() << "Error writing data to device: " << device->errorString();
    return;
  }
  if(wrote != d->payload.size())
    qWarning() << "Wrote " << wrote << "bytes, expected " << d->payload.size();
  //qDebug() << "Wrote " << wrote << "bytes, expected " << d->payload.size();
  //qDebug() << "Sent data: " << data;
}


void NetworkPacket::receiveFrom(QIODevice *device)
{
  int data_size;
  QDataStream s(device);
  s >> d->name >> data_size;
  while(device->bytesAvailable() < data_size)
    qApp->processEvents();
  //qDebug() << "reading " << data_size << " bytes";
  d->payload = device->read(data_size);
}


NetworkPacket::Type NetworkPacket::name() const
{
  return d->name;
}

void NetworkPacket::setName(const Type& name)
{
  d->name = name;
}

void NetworkPacket::setPayload(const QByteArray& payload)
{
  d->payload = payload;
}

QByteArray NetworkPacket::payload() const
{
  return d->payload;
}

void NetworkPacket::setPayload(const QVariant& payload)
{
  QBuffer buffer(&d->payload);
  buffer.open(QIODevice::WriteOnly);
  QDataStream s(&buffer);
  s << payload;
}

QVariant NetworkPacket::payloadVariant() const
{
  QBuffer buffer(&d->payload);
  buffer.open(QIODevice::ReadOnly);
  QVariant m;
  QDataStream s(&buffer);
  s >> m;
  return m;
}



QDebug operator<<(QDebug dbg, const NetworkPacket& packet)
{
  dbg << packet.d->name << packet.d->payload;
  return dbg;
}

NetworkPacket::ptr operator<<(const NetworkPacket::ptr& packet, const QByteArray& payload)
{
  packet->setPayload(payload);
  return packet;
}

NetworkPacket::ptr operator<<(const NetworkPacket::ptr& packet, const QVariant& payload)
{
  packet->setPayload(payload);
  return packet;
}

