/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
 * Copyright (C) 2017  Marco Gulino <marco@gulinux.net>
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

#ifndef PLANETARYIMAGER_H
#define PLANETARYIMAGER_H
#include <QObject>
#include "c++/dptr.h"
#include "drivers/driver.h"
#include "drivers/imager.h"
#include "image_handlers/imagehandler.h"
#include "image_handlers/saveimages.h"
#include "commons/configuration.h"

class PlanetaryImager : public QObject
{
  Q_OBJECT
public:
    typedef std::shared_ptr<PlanetaryImager> ptr;
    PlanetaryImager(
        const Driver::ptr &driver,
        const ImageHandler::ptr &imageHandler,
        const SaveImages::ptr &saveImages,
        Configuration &configuration
    );
    ~PlanetaryImager();
  Driver::Cameras cameras() const;
  Imager *imager() const;
  SaveImages::ptr saveImages() const;
  Configuration &configuration() const;

public slots:
  void scanCameras();
  void open(const Driver::Camera::ptr &camera);
  void closeImager();
  void startRecording();
  void setRecordingPaused(bool paused);
  void stopRecording();
  void quit();
signals:
  void camerasChanged();
  void cameraConnected();
  void cameraDisconnected();
private:
  DPTR
};

#endif // PLANETARYIMAGER_H
