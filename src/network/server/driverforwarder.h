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

#ifndef NETWORKDRIVER_H
#define NETWORKDRIVER_H

#include "c++/dptr.h"
#include "commons/fwd.h"
#include "network/networkreceiver.h"

FWD_PTR(PlanetaryImager)
FWD(Imager)
FWD_PTR(DriverForwarder)
FWD_PTR(NetworkDispatcher)

class DriverForwarder : public NetworkReceiver
{
public:
    typedef std::function<void(Imager *)> OnImagerChanged;
    DriverForwarder(const NetworkDispatcherPtr &dispatcher, const PlanetaryImagerPtr &planetaryImager);
    ~DriverForwarder();
    void getStatus(QVariantMap &status);
private:
  DPTR
};

#endif // NETWORKDRIVER_H
