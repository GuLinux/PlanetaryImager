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

#include "driverforwarder.h"
#include "network/protocol/driverprotocol.h"
using namespace std;

DPTR_IMPL(DriverForwarder) {
  Driver::ptr driver;
  Driver::Cameras cameras;
  Imager *imager = nullptr;
};

DriverForwarder::DriverForwarder(const NetworkDispatcher::ptr &dispatcher, const Driver::ptr& driver) : NetworkReceiver{dispatcher}, dptr(driver)
{
  register_handler(DriverProtocol::CameraList, [this, dispatcher](const NetworkPacket::ptr &p) {
      d->cameras = d->driver->cameras();
      dispatcher->send(DriverProtocol::sendCameraListReply(d->cameras));
  });
  register_handler(DriverProtocol::ConnectCamera, [this, dispatcher](const NetworkPacket::ptr &p) {
    delete d->imager;
    d->imager = nullptr;
    auto address = reinterpret_cast<Driver::Camera *>(p->property(DriverProtocol::CameraId).toLongLong());
    if(count_if(begin(d->cameras), end(d->cameras), [address](const Driver::Camera::ptr &p){ return p.get() == address; }) == 1) {
      d->imager = address->imager({}); // TODO: add handlers
    }
    dispatcher->send(DriverProtocol::packetConnectCameraReply()); // TODO: add status
  });
  register_handler(DriverProtocol::GetCameraName, [this, dispatcher](const NetworkPacket::ptr &p) {
   dispatcher->send(DriverProtocol::packetGetCameraNameReply() << DriverProtocol::propertyCameraName(d->imager->name()));
  });
}

DriverForwarder::~DriverForwarder()
{
}

