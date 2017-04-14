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

#include "remotefilesystembrowser.h"
#include "network/client/remotefilesystem.h"
#include "pickdirectory.h"

using namespace std;

DPTR_IMPL(RemoteFilesystemBrowser) {
  RemoteFilesystem::ptr filesystem;
};

RemoteFilesystemBrowser::RemoteFilesystemBrowser(const NetworkDispatcher::ptr& dispatcher) 
  : dptr(make_shared<RemoteFilesystem>(dispatcher))
{
}

RemoteFilesystemBrowser::~RemoteFilesystemBrowser()
{
}

void RemoteFilesystemBrowser::pickDirectory(const QString currentDirectory) const
{
  auto pick_directory = new PickDirectory(d->filesystem, currentDirectory);
  connect(pick_directory, &PickDirectory::directoryPicked, this, &FilesystemBrowser::directoryPicked);
  connect(pick_directory, &PickDirectory::finished, pick_directory, &QDialog::deleteLater);
  pick_directory->show();
}
