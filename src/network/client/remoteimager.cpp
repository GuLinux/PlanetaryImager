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

#include "remoteimager.h"
#include "network/protocol/driverprotocol.h"
using namespace std;

DPTR_IMPL(RemoteImager) {
  const ImageHandler::ptr image_handler;
  QString name;
};

RemoteImager::RemoteImager(qlonglong id, const ImageHandler::ptr& image_handler, const NetworkDispatcher::ptr &dispatcher) : Imager{image_handler}, NetworkReceiver{dispatcher}, dptr(image_handler)
{
  register_handler(DriverProtocol::ConnectCameraReply, [](const NetworkPacket::ptr &) {});
  register_handler(DriverProtocol::GetCameraNameReply, [this](const NetworkPacket::ptr &packet) {
    d->name = packet->property("name").toString();
  });

  dispatcher->queue_send( DriverProtocol::packetConnectCamera() << NetworkPacket::Property{DriverProtocol::CameraId, id} );
  wait_for_processed(DriverProtocol::ConnectCameraReply);
  dispatcher->queue_send(DriverProtocol::packetGetCameraName() );
  wait_for_processed(DriverProtocol::GetCameraNameReply);
}

RemoteImager::~RemoteImager()
{
}


void RemoteImager::startLive()
{
}

void RemoteImager::clearROI()
{
}

Imager::Controls RemoteImager::controls() const
{
  return {};
}

QString RemoteImager::name() const
{
  return d->name;
}

Imager::Properties RemoteImager::properties() const
{
  return {};
}

void RemoteImager::setControl(const Imager::Control& control)
{
}

void RemoteImager::setROI(const QRect&)
{
}



#include "remoteimager.moc"
