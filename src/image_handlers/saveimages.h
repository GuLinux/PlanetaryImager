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

#ifndef SAVEIMAGE_H
#define SAVEIMAGE_H

#include "image_handlers/imagehandler.h"
#include "drivers/imager.h"
#include <QObject>

#include "commons/configuration.h"
class SaveImages : public QObject, public ImageHandler
{
  Q_OBJECT
public:
  typedef std::shared_ptr<SaveImages> ptr;
  class Error;
    SaveImages(QObject *parent = 0);
    virtual ~SaveImages();

public slots:
  virtual void startRecording(Imager *imager) = 0;
  virtual void endRecording() = 0;
  virtual void setPaused(bool paused) = 0;
signals:
  void saveFPS(double fps);
  void meanFPS(double fps);
  void savedFrames(long frames);
  void droppedFrames(long frames);
  void recording(const QString &filename);
  void finished();
};

class SaveImages::Error : public std::runtime_error {
public:
  Error(const QString &what);
  static Error openingFile(const QString &file, const QString &additionalInfo = {});
};

#endif // SAVEIMAGE_H
