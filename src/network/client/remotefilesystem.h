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

#ifndef REMOTEFILESYSTEM_H
#define REMOTEFILESYSTEM_H

#include "network/networkdispatcher.h"
#include "c++/dptr.h"
#include <QList>

class RemoteFilesystem;
class FilesystemEntry {
public:
  typedef std::shared_ptr<FilesystemEntry> ptr;
  typedef QList<ptr> List;
  enum Type { File, Directory };
  FilesystemEntry(const QString &name, const QString &path, Type type, const std::shared_ptr<RemoteFilesystem> &filesystem);
  ~FilesystemEntry();
  QString name() const;
  QString path() const;
  Type type() const;
  ptr parent() const;
  List children() const;
  bool isRoot() const;
private:
  DPTR
};

class RemoteFilesystem : public NetworkReceiver, public std::enable_shared_from_this<RemoteFilesystem>
{
public:
  typedef std::shared_ptr<RemoteFilesystem> ptr;
  RemoteFilesystem(const NetworkDispatcher::ptr &dispatcher);
  ~RemoteFilesystem();
  
  FilesystemEntry::ptr entry(const QString &path);
  FilesystemEntry::List entries(const QString &parent_path);
private:
  DPTR
};

#endif // REMOTEFILESYSTEM_H
