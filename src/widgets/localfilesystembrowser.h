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

#ifndef LOCALFILESYSTEMBROWSER_H
#define LOCALFILESYSTEMBROWSER_H

#include "commons/filesystembrowser.h"
#include "c++/dptr.h"

class LocalFilesystemBrowser : public FilesystemBrowser
{
  Q_OBJECT
public:
  LocalFilesystemBrowser(QObject *parent = nullptr);
  ~LocalFilesystemBrowser();
  bool isLocal() const override { return true; }
public slots:
  void pickDirectory(const QString currentDirectory = {}) const override;
private:
  DPTR
};

#endif // LOCALFILESYSTEMBROWSER_H
