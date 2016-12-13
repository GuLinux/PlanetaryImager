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

using namespace std;

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
  FilesystemEntry::ptr entry(const QFileInfo &info);
};


RemoteFilesystem::RemoteFilesystem(const NetworkDispatcher::ptr& dispatcher) : NetworkReceiver{dispatcher}, dptr(this)
{
}

RemoteFilesystem::~RemoteFilesystem()
{
}
FilesystemEntry::ptr RemoteFilesystem::entry(const QString& path)
{
  qDebug() << "Entry for " << path;
  return d->entry({path});
}

FilesystemEntry::List RemoteFilesystem::entries(const QString& parent_path)
{
  qDebug() << "Entries for " << parent_path;
  FilesystemEntry::List entries;
  QDir dir(parent_path);
  for(auto file: dir.entryInfoList()) {
    entries.push_back(d->entry(file));
    qDebug() << file.fileName() << ", " <<  file.canonicalFilePath();
  }
  return entries;
}

FilesystemEntry::ptr RemoteFilesystem::Private::entry(const QFileInfo& info)
{
  return make_shared<FilesystemEntry>(info.fileName(), info.canonicalFilePath(), info.isDir() ? FilesystemEntry::Directory : FilesystemEntry::File, q->shared_from_this());
}
