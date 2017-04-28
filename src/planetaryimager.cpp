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

#include "planetaryimager.h"
#include "Qt/functional.h"

DPTR_IMPL(PlanetaryImager) {
  Driver::ptr driver;
  ImageHandler::ptr imageHandler;
  SaveImages::ptr saveImages;
  Configuration::ptr configuration;
  PlanetaryImager *q;
  
  Driver::Cameras cameras;
  Imager *imager = nullptr;
};

PlanetaryImager::PlanetaryImager(
  const Driver::ptr &driver,
  const ImageHandler::ptr &imageHandler,
  const SaveImages::ptr &saveImages,
  const Configuration::ptr &configuration
) : QObject{}, dptr(driver, imageHandler, saveImages, configuration, this)
{

}

PlanetaryImager::~PlanetaryImager()
{
}

Imager * PlanetaryImager::imager() const
{
  return d->imager;
}

Driver::Cameras PlanetaryImager::cameras() const
{
  return d->cameras;
}

void PlanetaryImager::record()
{
  d->saveImages->startRecording(d->imager);
}

void PlanetaryImager::setRecordingPaused(bool paused)
{
  d->saveImages->setPaused(paused);
}

void PlanetaryImager::stopRecording()
{
  d->saveImages->endRecording();
}

void PlanetaryImager::scanCameras()
{
  GuLinux::qAsyncR<Driver::Cameras>([this] { return d->driver->cameras(); }, [this](const Driver::Cameras &cameras) {
    d->cameras = cameras;
    emit camerasChanged();
  }, this);
}

void PlanetaryImager::open(const Driver::Camera::ptr& camera)
{
}


