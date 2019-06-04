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
  Cameras cameras;
  DriverProtocol::DriverStatus status;
};

class RemoteCamera : public Driver::Camera {
public:
  RemoteCamera(const QString &name, qlonglong address, const NetworkDispatcher::ptr &dispatcher) : _name{name}, _address{address}, _dispatcher{dispatcher} {}
  Imager * imager(const ImageHandlerPtr & imageHandler) const override;
  QString name() const override { return _name; }
private:
  const QString _name;
  const qlonglong _address;
  const NetworkDispatcher::ptr _dispatcher;
};

Imager * RemoteCamera::imager(const ImageHandlerPtr& imageHandler) const
{
  return new RemoteImager{imageHandler, _dispatcher, _address};
}


RemoteDriver::RemoteDriver(const NetworkDispatcher::ptr &dispatcher) : NetworkReceiver{dispatcher}, dptr()
{
  d->status.imager_running = false;
  register_handler(DriverProtocol::CameraListReply, [this](const NetworkPacket::ptr &packet){
    qDebug() << "Processing cameras: " << packet;
    d->cameras.clear();
    DriverProtocol::decode(d->cameras, packet, [&](const QString &name, qlonglong address) { return make_shared<RemoteCamera>(name, address, this->dispatcher() ); });
  });
  register_handler(NetworkProtocol::HelloReply, [this](const NetworkPacket::ptr &p) {
    qDebug() << "hello reply handler";
    d->status = DriverProtocol::decodeStatus(p);
  });
}

RemoteDriver::~RemoteDriver()
{
}


Driver::Cameras RemoteDriver::cameras() const
{
  dispatcher()->queue_send(DriverProtocol::packetCameraList() );
  wait_for_processed(DriverProtocol::CameraListReply);
  return d->cameras;;
}

Driver::Camera::ptr RemoteDriver::existing_running_camera() const
{
  if(! d->status.imager_running)
    return {};
  return make_shared<RemoteCamera>(QString{}, -1l, dispatcher());
}


