/*
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

#include "filesystemprotocol.h"
#include <QFileInfo>
#include "network/networkpacket.h"

using namespace std;
using namespace std::placeholders;
PROTOCOL_NAME_VALUE(Filesystem, FileInfo);
PROTOCOL_NAME_VALUE(Filesystem, FileInfoReply);
PROTOCOL_NAME_VALUE(Filesystem, Children);
PROTOCOL_NAME_VALUE(Filesystem, ChildrenReply);

namespace {
  QVariantMap fileInfo2Map(const QFileInfo &fileInfo) {
    return {
      {"name", fileInfo.fileName()},
      {"path", fileInfo.canonicalFilePath()},
      {"isFile", fileInfo.isFile()},
      {"isDir", fileInfo.isDir()},
    };
  }
  void decodeFileInfo(const QVariantMap &m, FilesystemProtocol::CreateFileInfo &createFileInfo) {
    createFileInfo(m["name"].toString(), m["path"].toString(), m["isFile"].toBool(), m["isDir"].toBool());
  }
}

NetworkPacketPtr FilesystemProtocol::childrenReply(const QList<QFileInfo>& filesInfo)
{
  QVariantList list;
  transform(filesInfo.begin(), filesInfo.end(), back_inserter(list), fileInfo2Map);
  return packetChildrenReply() << list;
}

NetworkPacketPtr FilesystemProtocol::fileInfoReply(const QFileInfo& fileInfo)
{
  return packetFileInfoReply() << fileInfo2Map(fileInfo);
}

void FilesystemProtocol::decodeFileInfoReply(const NetworkPacketPtr& packet, CreateFileInfo createFileInfo)
{
  qDebug() << packet->payloadVariant().toMap();
  decodeFileInfo(packet->payloadVariant().toMap(), createFileInfo);
}

void FilesystemProtocol::decodeChildrenReply(const NetworkPacketPtr& packet, CreateFileInfo createFileInfo)
{
  auto l = packet->payloadVariant().toList();
  for(auto v: l) {
    qDebug() << v.toMap();
    decodeFileInfo(v.toMap(), createFileInfo);
  }
}

