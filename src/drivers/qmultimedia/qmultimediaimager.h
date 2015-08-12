/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#ifndef QMULTIMEDIAIMAGER_H
#define QMULTIMEDIAIMAGER_H

#include "drivers/imager.h"
#include "dptr.h"
class QCameraInfo;
class QMultimediaImager : public Imager
{
    Q_OBJECT

public:
  QMultimediaImager(const QCameraInfo &cameraInfo);
  ~QMultimediaImager();
  virtual Imager::Chip chip() const;
  virtual QString name() const;
  virtual Imager::Settings settings() const;
public slots:
  virtual void setSetting(const Setting &setting);
  virtual void startLive();
  virtual void stopLive();
private:
  D_PTR
};

#endif // QMULTIMEDIAIMAGER_H
