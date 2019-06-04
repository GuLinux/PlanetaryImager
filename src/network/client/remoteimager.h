/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
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

#ifndef REMOTEIMAGER_H
#define REMOTEIMAGER_H
#include "drivers/imager.h"
#include "c++/dptr.h"
#include "commons/fwd.h"
#include "network/networkreceiver.h"

FWD_PTR(NetworkDispatcher)

class RemoteImager : public Imager, public NetworkReceiver
{
Q_OBJECT
public:
  RemoteImager(const ImageHandlerPtr &image_handler, const NetworkDispatcherPtr &dispatcher, qlonglong id = -1);
  ~RemoteImager();
  Controls controls() const override;  
  QString name() const override;
  Properties properties() const override;
public slots:
  void setROI(const QRect &roi) override;
  void clearROI() override;
  void setControl(const Imager::Control &control) override;
  void startLive() override;
private:
  DPTR
};

#endif // REMOTEIMAGER_H
