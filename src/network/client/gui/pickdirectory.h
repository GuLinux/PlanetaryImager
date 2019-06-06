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

#ifndef PICKDIRECTORY_H
#define PICKDIRECTORY_H

#include <QDialog>
#include "c++/dptr.h"
#include "commons/fwd.h"

FWD_PTR(RemoteFilesystem)

class PickDirectory : public QDialog
{
    Q_OBJECT
public:
  PickDirectory(const RemoteFilesystemPtr &filesystem, const QString &startingDirectory);
  ~PickDirectory();
signals:
  void directoryPicked(const QString &path);
private:
  DPTR
};

#endif // PICKDIRECTORY_H
