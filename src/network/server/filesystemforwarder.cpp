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

#include "network/server/filesystemforwarder.h"
#include "network/protocol/filesystemprotocol.h"
#include "network/networkdispatcher.h"
#include "network/networkpacket.h"
#include <QFileInfo>
#include <QDir>
#include "commons/utils.h"

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(FilesystemForwarder) {
  FilesystemForwarder *q;
  void entry(const NetworkPacketPtr &packet);
  void listChildren(const NetworkPacketPtr &packet);
};

FilesystemForwarder::FilesystemForwarder(const NetworkDispatcherPtr& dispatcher) : NetworkReceiver{dispatcher}, dptr(this)
{
  register_handler(FilesystemProtocol::FileInfo, bind(&Private::entry, d.get(), _1));
  register_handler(FilesystemProtocol::Children, bind(&Private::listChildren, d.get(), _1));
}

FilesystemForwarder::~FilesystemForwarder()
{
}

void FilesystemForwarder::Private::entry(const NetworkPacketPtr& packet)
{
  LOG_F_SCOPE
  QFileInfo info{packet->payloadVariant().toString()};
  q->dispatcher()->send(FilesystemProtocol::fileInfoReply(info));
}

void FilesystemForwarder::Private::listChildren(const NetworkPacketPtr& packet)
{
  LOG_F_SCOPE
  QDir dir{packet->payloadVariant().toString()};
  q->dispatcher()->send(FilesystemProtocol::childrenReply(dir.entryInfoList()));
}
