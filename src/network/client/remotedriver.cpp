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

#include "remotedriver.h"
#include "network/protocol/driverprotocol.h"
#include <QCoreApplication>
using namespace std;

DPTR_IMPL(RemoteDriver) {
  NetworkDispatcher::ptr dispatcher;
  Cameras cameras;
  bool cameras_processed;
};

RemoteDriver::RemoteDriver(const NetworkDispatcher::ptr &dispatcher) : dptr(dispatcher)
{
  dispatcher->attach(this);
}

RemoteDriver::~RemoteDriver()
{
  d->dispatcher->detach(this);
}


Driver::Cameras RemoteDriver::cameras() const
{
  auto packet = make_shared<NetworkPacket>();
  packet->setName(DriverProtocol::CameraList);
  d->cameras_processed = false;
  QMetaObject::invokeMethod(d->dispatcher.get(), "send", Q_ARG(NetworkPacket::ptr, packet));
  while(! d->cameras_processed)
    qApp->processEvents();
  return d->cameras;;
}


void RemoteDriver::handle(const NetworkPacket::ptr& packet)
{
  if(packet->name() != DriverProtocol::CameraListReply)
    return;
  qDebug() << "Processing cameras: " << packet;
  d->cameras.clear();
  DriverProtocol::decode(d->cameras, packet, d->dispatcher);
  d->cameras_processed = true;
}
