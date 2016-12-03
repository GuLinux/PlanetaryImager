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

#ifndef DRIVERPROTOCOL_H
#define DRIVERPROTOCOL_H
#include <QString>
#include "drivers/driver.h"
#include "network/networkpacket.h"
#include "network/networkdispatcher.h"
class DriverProtocol {
public:
  static const QString CameraList;
  static const QString CameraListReply;
  static const QString CamerasParameter;
  static const QString ConnectCamera;
  static void encode(const Driver::Cameras &cameras, const NetworkPacket::ptr &packet);
  static void decode(Driver::Cameras &cameras, const NetworkPacket::ptr &packet, const NetworkDispatcher::ptr dispatcher);
};

#endif // DRIVERPROTOCOL_H
