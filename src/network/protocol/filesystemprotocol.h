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

#ifndef FILESYSTEMPROTOCOL_H
#define FILESYSTEMPROTOCOL_H
#include "network/protocol/protocol.h"
#include <QList>

class QFileInfo;

class FilesystemProtocol : public NetworkProtocol
{
public:
  ADD_PROTOCOL_PACKET_NAME(FileInfo)
  ADD_PROTOCOL_PACKET_NAME(FileInfoReply)
  ADD_PROTOCOL_PACKET_NAME(Children)
  ADD_PROTOCOL_PACKET_NAME(ChildrenReply)
  
  typedef std::function<void(const QString &name, const QString &path, bool isFile, bool isDir)> CreateFileInfo;
  
  static NetworkPacket::ptr fileInfoReply(const QFileInfo &fileInfo);
  static void decodeFileInfoReply(const NetworkPacket::ptr &packet, CreateFileInfo createFileInfo);
  static NetworkPacket::ptr childrenReply(const QList<QFileInfo> &filesInfo);
  static void decodeChildrenReply(const NetworkPacket::ptr &packet, CreateFileInfo createFileInfo);
};

#endif // FILESYSTEMPROTOCOL_H
