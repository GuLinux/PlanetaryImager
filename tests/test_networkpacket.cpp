/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
 * Copyright (C) 2017  Marco Gulino <marco@gulinux.net>
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
 */

#include "gtest/gtest.h"
#include "network/networkpacket.h"
#include <QBuffer>

QBuffer *writeBuffer() {
  auto buffer = new QBuffer();
  buffer->open(QIODevice::WriteOnly);
  return buffer;
}

QBuffer *readBuffer(const QByteArray &data) {
  auto buffer = new QBuffer();
  buffer->setData(data);
  buffer->open(QIODevice::ReadOnly);
  return buffer;
}

QByteArray emptyPayloadData(const QString &name) {
  QByteArray data(name.toLatin1());
  data.prepend(static_cast<char>(name.size()));
  data.append("\0\0\0\0");
  return data;
}

TEST(TestNetworkPacket, testPayloadEncode)
{
  QString name = "HelloPacket";
  auto expected = emptyPayloadData(name);

  NetworkPacket packet(name);
  auto buffer = writeBuffer();
  packet.sendTo(buffer);
  ASSERT_EQ(expected, buffer->data());
}


TEST(TestNetworkPacket, testPayloadDecode)
{
  QString name = "AnotherPacketName";
  auto buffer = readBuffer(emptyPayloadData(name));
  NetworkPacket packet;
  packet.receiveFrom(buffer);
  ASSERT_EQ(name, packet.name());
  ASSERT_EQ(0, packet.payload().size());
}

TEST(TestNetworkPacket, testPacketEncode)
{
  auto expected = emptyPayloadData("hello");
  QByteArray payload("abcd123");
  expected[expected.size() - 1] = static_cast<char>(payload.size());
  expected.append(payload);
  
  NetworkPacket packet("hello");
  packet.setPayload(payload);
  auto buffer = writeBuffer();
  packet.sendTo(buffer);
  
  ASSERT_EQ(expected, buffer->data());
}


TEST(TestNetworkPacket, testPacketEncodeWithLongerPayload)
{
  auto expected = emptyPayloadData("hello");
  QByteArray payload("abcd123");
  expected[expected.size() - 1] = static_cast<char>(payload.size());
  expected.append(payload);
  
  NetworkPacket packet("hello");
  packet.setPayload(payload);
  auto buffer = writeBuffer();
  packet.sendTo(buffer);
  
  ASSERT_EQ(expected, buffer->data());
}

TEST(TestNetworkPacket, testPacketDecode)
{
  auto data = emptyPayloadData("hello");
  QByteArray expected_payload("x", 200);
  data[data.size() - 1] = static_cast<char>(expected_payload.size());
  data.append(expected_payload);
  
  NetworkPacket packet;
  auto buffer = readBuffer(data);
  packet.receiveFrom(buffer);
  
  ASSERT_EQ("hello", packet.name());
  ASSERT_EQ(expected_payload, packet.payload());
}



TEST(TestNetworkPacket, testPacketDecodeWithLongerPayload)
{
  auto data = emptyPayloadData("hello");
  QByteArray expected_payload("a", 200);
  data[data.size() - 1] = static_cast<char>(expected_payload.size());
  data.append(expected_payload);
  
  NetworkPacket packet;
  auto buffer = readBuffer(data);
  packet.receiveFrom(buffer);
  
  ASSERT_EQ("hello", packet.name());
  ASSERT_EQ(expected_payload, packet.payload());
}
