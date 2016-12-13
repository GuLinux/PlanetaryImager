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

#include "remotefilesystem.h"
#include <QDebug>
#include "network/protocol/filesystemprotocol.h"
#include "commons/utils.h"

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(FilesystemEntry) {
  const QString name;
  const QString path;
  const Type type;
  RemoteFilesystem::ptr filesystem;
  List children;
  ptr parent;
  bool children_retrieved = false;
  void retrieve_children();
  void retrieve_parent();
};

FilesystemEntry::FilesystemEntry(const QString &name, const QString &path, Type type, const RemoteFilesystem::ptr& filesystem) : dptr(name, path, type, filesystem)
{
}
FilesystemEntry::~FilesystemEntry()
{
}
QString FilesystemEntry::name() const
{
  return d->name;
}
QString FilesystemEntry::path() const
{
  return d->path;
}

FilesystemEntry::Type FilesystemEntry::type() const
{
  return d->type;
}

bool FilesystemEntry::isRoot() const
{
  return d->path == "/";
}


FilesystemEntry::ptr FilesystemEntry::parent() const
{
  if(! isRoot() && ! d->parent)
    d->retrieve_parent();
  return d->parent;
}

FilesystemEntry::List FilesystemEntry::children() const
{
  if(d->type == Directory && ! d->children_retrieved)
    d->retrieve_children();
  return d->children;
}

void FilesystemEntry::Private::retrieve_children()
{
  children = filesystem->entries(path);
}

void FilesystemEntry::Private::retrieve_parent()
{
  auto components = path.split('/');
  components.removeLast();
  auto parent_path = components.join('/');
  parent = filesystem->entry(parent_path);
}



// TODO: replace with remote
#include <QFileInfo>
#include <QDir>
DPTR_IMPL(RemoteFilesystem) {
  RemoteFilesystem *q;
  typedef function<void(const FilesystemEntry::ptr &)> OnEntry;
  void entry(OnEntry onEntry, const QString &name, const QString &path, bool isFile, bool isDir);
  FilesystemEntry::ptr last_entry;
  FilesystemEntry::List last_list;
  void fileInfoReply(const NetworkPacket::ptr &p);
  void childrenReply(const NetworkPacket::ptr &p);
};


RemoteFilesystem::RemoteFilesystem(const NetworkDispatcher::ptr& dispatcher) : NetworkReceiver{dispatcher}, dptr(this)
{
  register_handler(FilesystemProtocol::FileInfoReply, bind(&Private::fileInfoReply, d.get(), _1));
  register_handler(FilesystemProtocol::ChildrenReply, bind(&Private::childrenReply, d.get(), _1));
}

RemoteFilesystem::~RemoteFilesystem()
{
}
FilesystemEntry::ptr RemoteFilesystem::entry(const QString& path)
{
  LOG_F_SCOPE
  d->last_entry = nullptr;
  dispatcher()->send(FilesystemProtocol::packetFileInfo() << QVariant{path});
  wait_for_processed(FilesystemProtocol::FileInfoReply);
  return d->last_entry;
}

FilesystemEntry::List RemoteFilesystem::entries(const QString& parent_path)
{
  LOG_F_SCOPE
  d->last_list = {};
  dispatcher()->send(FilesystemProtocol::packetChildren() << QVariant{parent_path});
  wait_for_processed(FilesystemProtocol::ChildrenReply);
  return d->last_list;
}

void RemoteFilesystem::Private::entry(OnEntry onEntry, const QString& name, const QString& path, bool isFile, bool isDir)
{
  onEntry(make_shared<FilesystemEntry>(name, path, isDir ? FilesystemEntry::Directory : FilesystemEntry::File, q->shared_from_this()));
}

void RemoteFilesystem::Private::fileInfoReply(const NetworkPacket::ptr& p)
{
  LOG_F_SCOPE
  FilesystemProtocol::decodeFileInfoReply(p, bind(&Private::entry, this, [=](const FilesystemEntry::ptr &e){ last_entry = e; }, _1, _2, _3, _4));
}

void RemoteFilesystem::Private::childrenReply(const NetworkPacket::ptr& p)
{
  LOG_F_SCOPE
  FilesystemProtocol::decodeChildrenReply(p, bind(&Private::entry, this, [=](const FilesystemEntry::ptr &e){ last_list.push_back(e);}, _1, _2, _3, _4));
}


