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
#ifndef CAMERASETTINGSWIDGET_H
#define CAMERASETTINGSWIDGET_H

#include <QWidget>
#include "dptr.h"
#include "drivers/imager.h"
#include "commons/configuration.h"
#include "commons/fwd.h"

FWD_PTR(FilesystemBrowser)

class CameraControlsWidget : public QWidget
{
public:
~CameraControlsWidget();
CameraControlsWidget(Imager *imager, Configuration &configuration, const FilesystemBrowserPtr &filesystemBrowser, QWidget* parent = 0);

private:
    DPTR
};

#endif // CAMERASETTINGSWIDGET_H
