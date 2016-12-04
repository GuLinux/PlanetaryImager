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
#include "remoteimager.h"
using namespace std;

DPTR_IMPL(RemoteDriver) {
  NetworkDispatcher::ptr dispatcher;
  Cameras cameras;
};

class RemoteCamera : public Driver::Camera {
public:
  RemoteCamera(const QString &name, qlonglong address, const NetworkDispatcher::ptr &dispatcher) : _name{name}, _address{address}, _dispatcher{dispatcher} {}
  Imager * imager(const ImageHandler::ptr & imageHandler) const override;
  QString name() const override { return _name; }
private:
  const QString _name;
  const qlonglong _address;
  const NetworkDispatcher::ptr _dispatcher;
};

Imager * RemoteCamera::imager(const ImageHandler::ptr& imageHandler) const
{
  return new RemoteImager{_address, imageHandler, _dispatcher};
}


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
  QMetaObject::invokeMethod(d->dispatcher.get(), "send", Q_ARG(NetworkPacket::ptr, packet));
  wait_for_processed(DriverProtocol::CameraListReply);
  return d->cameras;;
}


void RemoteDriver::handle(const NetworkPacket::ptr& packet)
{
  if(packet->name() != DriverProtocol::CameraListReply)
    return;
  qDebug() << "Processing cameras: " << packet;
  d->cameras.clear();
  DriverProtocol::decode(d->cameras, packet, [&](const QString &name, qlonglong address) { return make_shared<RemoteCamera>(name, address, d->dispatcher); });
  set_processed(DriverProtocol::CameraListReply);
}
