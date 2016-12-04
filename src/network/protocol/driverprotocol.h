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
#include "drivers/driver.h"
#include "network/protocol/protocol.h"
#include <QList>
class DriverProtocol : public NetworkProtocol {
public:
  ADD_PROTOCOL_NAME(CameraList)
  ADD_PROTOCOL_NAME(CameraListReply)
  ADD_PROTOCOL_NAME(CamerasParameter)
  ADD_PROTOCOL_NAME(ConnectCamera)
  ADD_PROTOCOL_NAME(CameraId)
  ADD_PROTOCOL_NAME(ConnectCameraReply)
  ADD_PROTOCOL_NAME(GetCameraName)
  ADD_PROTOCOL_NAME(GetCameraNameReply)
  static void encode(const Driver::Cameras &cameras, const NetworkPacket::ptr &packet);
  typedef std::function<Driver::Camera::ptr(const QString &, qlonglong)> CameraFactory;
  static void decode(Driver::Cameras &cameras, const NetworkPacket::ptr &packet, const CameraFactory &factory);
};

#endif // DRIVERPROTOCOL_H
