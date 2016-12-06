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
  QString name;
  QVariantMap properties;
  QByteArray getBinaryData();
  void fromBinaryData(const QByteArray &ba);
};

NetworkPacket::NetworkPacket() : dptr()
{
}

NetworkPacket::NetworkPacket(const NameType& name) : NetworkPacket()
{
  setName(name);
}


NetworkPacket::~NetworkPacket()
{
}

QByteArray NetworkPacket::Private::getBinaryData()
{
  QBuffer buf;
  buf.open(QIODevice::ReadWrite);
  QDataStream s(&buf);
  s << properties;
  return buf.data();
}


void NetworkPacket::sendTo(QIODevice *device) const
{
  QByteArray data = d->getBinaryData();
  if(d->properties.count("frame")) {
    qDebug() << "data size: " << data.size() << ", original frame data size: " << d->properties["frame"].toByteArray().size();
  }
  QDataStream s(device);
  s << d->name << data.size();
  qint64 wrote = device->write(data);
  if(wrote != data.size())
    qWarning() << "Wrote " << wrote << "bytes, expected " << data.size();
  qDebug() << "Wrote " << wrote << "bytes, expected " << data.size();
  //qDebug() << "Sent data: " << data;
}

void NetworkPacket::Private::fromBinaryData(const QByteArray& ba)
{
  QBuffer buf;
  buf.setData(ba);
  buf.open(QIODevice::ReadWrite);
  QDataStream s(&buf);
  s >> properties;
}

void NetworkPacket::receiveFrom(QIODevice *device)
{
  int data_size;
  QDataStream s(device);
  s >> d->name >> data_size;
  while(device->bytesAvailable() < data_size)
    qApp->processEvents();
  qDebug() << "reading " << data_size << " bytes";
  d->fromBinaryData(device->read(data_size));

}

NetworkPacket * NetworkPacket::setProperty(const KeyType& property, const QVariant& value)
{
  d->properties[property] = value;
  return this;
}

QVariant NetworkPacket::property(const KeyType& name) const
{
  return d->properties[name];
}

NetworkPacket::NameType NetworkPacket::name() const
{
  return d->name;
}

NetworkPacket *NetworkPacket::setName(const NameType& name)
{
  d->name = name;
  return this;
}

NetworkPacket::ptr operator<<(NetworkPacket::ptr packet, const NetworkPacket::Property& property)
{
  packet->setProperty(property.key, property.value);
  return packet;
}



QDebug operator<<(QDebug dbg, const NetworkPacket& packet)
{
  dbg << packet.d->properties;
  return dbg;
}
