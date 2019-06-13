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

#include "network/networkpacket.h"
#include <QIODevice>
#include <QDataStream>
#include <QJsonDocument>
#include "Qt/qt_strings_helper.h"
#include <QCoreApplication>
#include <QBuffer>

using namespace std;

DPTR_IMPL(NetworkPacket) {
  NetworkPacketType name;
  QByteArray payload;
  static const int NAME_BYTES = 1;
  static const int PACKET_BYTES = 4;
  template<typename T> QByteArray asFixedBytes(T number, int digits) const;
  template<typename T> T fromFixedBytes(const QByteArray &bytes) const;
  
  QByteArray chunked_read(int bytes, QIODevice *device, int chunk_size = 1024);
};

NetworkPacket::NetworkPacket() : dptr()
{
}

NetworkPacket::NetworkPacket(const NetworkPacketType& name) : NetworkPacket()
{
  if(name.size() > 255) {
    throw runtime_error(("Packet name must be under 256 characters (was: %1)" % name).toStdString());
  }
  setName(name);
}

template<typename T> QByteArray NetworkPacket::Private::asFixedBytes(T number, int digits) const
{
  QByteArray out(digits, '\0');
  for(int i = digits-1; i >=0; i--) {
    out[i] = number % 256;
    number /= 256;
  }
  return out;
}

template<typename T> T NetworkPacket::Private::fromFixedBytes(const QByteArray &bytes) const
{
  T number = 0;
  for(uint8_t b: bytes) {
    number *= 256;
    number += static_cast<T>(b);
  }
  return number;
}



NetworkPacket::~NetworkPacket()
{
}



qint64 NetworkPacket::sendTo(QIODevice *device) const
{
  qint64 wrote = device->write( d->asFixedBytes(d->name.size(), Private::NAME_BYTES));
  wrote += device->write(d->name.toLatin1());
  wrote += device->write(d->asFixedBytes(d->payload.size(), Private::PACKET_BYTES));
  wrote += device->write(d->payload);
  auto expected = d->name.size() + d->payload.size() + Private::PACKET_BYTES + Private::NAME_BYTES;
  
  if(wrote != expected)
    qWarning() << "Wrote " << wrote << "bytes, expected " << d->payload.size();
  return wrote;
}


void NetworkPacket::receiveFrom(QIODevice *device)
{
  auto name_size = d->fromFixedBytes<int>(d->chunked_read(Private::NAME_BYTES, device));
  auto name = d->chunked_read(name_size, device);
  d->name = QString::fromLatin1(name);
  
  auto data_size = d->fromFixedBytes<int>(d->chunked_read(Private::PACKET_BYTES, device));
  d->payload = d->chunked_read(data_size, device);
}

QByteArray NetworkPacket::Private::chunked_read(int bytes, QIODevice *device, int chunk_size)
{
  QByteArray dest;
  while(dest.size() < bytes) {
    auto remaining = bytes - dest.size();
    auto chunk = std::min(remaining, chunk_size);
    while(device->bytesAvailable() < chunk)
      qApp->processEvents();
    dest.append(device->read(chunk));
  }
  return dest;
}



NetworkPacketType NetworkPacket::name() const
{
  return d->name;
}

void NetworkPacket::setName(const NetworkPacketType& name)
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


void NetworkPacket::movePayload(QByteArray && payload)
{
  d->payload = std::move(payload);
}


QDebug operator<<(QDebug dbg, const NetworkPacket& packet)
{
  dbg << packet.d->name << packet.d->payload;
  return dbg;
}

NetworkPacketPtr operator<<(const NetworkPacketPtr& packet, const QByteArray& payload)
{
  packet->setPayload(payload);
  return packet;
}

NetworkPacketPtr operator<<(const NetworkPacketPtr& packet, const QVariant& payload)
{
  packet->setPayload(payload);
  return packet;
}

