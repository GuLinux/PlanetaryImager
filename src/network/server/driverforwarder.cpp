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
  NetworkDispatcher::ptr dispatcher;
  Driver::ptr driver;
  Driver::Cameras cameras;
};

DriverForwarder::DriverForwarder(const NetworkDispatcher::ptr &dispatcher, const Driver::ptr& driver) : dptr(dispatcher, driver)
{
  dispatcher->attach(this);
}

DriverForwarder::~DriverForwarder()
{
  d->dispatcher->detach(this);
}


void DriverForwarder::handle(const NetworkPacket::ptr& packet)
{
  if(packet->name() != DriverProtocol::CameraList)
    return;
  d->cameras = d->driver->cameras();
  auto reply = make_shared<NetworkPacket>();
  reply->setName(DriverProtocol::CameraListReply);
  DriverProtocol::encode(d->cameras, reply);

  d->dispatcher->send(reply);
  qDebug() << "Got packet: " << packet;
}
