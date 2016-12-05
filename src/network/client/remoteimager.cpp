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
#include <QDebug>

using namespace std;

DPTR_IMPL(RemoteImager) {
  const ImageHandler::ptr image_handler;
  QString name;
  Properties properties;
  Controls controls;
};

RemoteImager::RemoteImager(qlonglong id, const ImageHandler::ptr& image_handler, const NetworkDispatcher::ptr &dispatcher) : Imager{image_handler}, NetworkReceiver{dispatcher}, dptr(image_handler)
{
  register_handler(DriverProtocol::ConnectCameraReply, [](const NetworkPacket::ptr &) {});
  register_handler(DriverProtocol::GetCameraNameReply, [this](const NetworkPacket::ptr &packet) {
    d->name = packet->property(DriverProtocol::CameraName).toString();
  });
  register_handler(DriverProtocol::GetPropertiesReply, [this](const NetworkPacket::ptr &packet) {
    DriverProtocol::decode(d->properties, packet);
  });
  register_handler(DriverProtocol::GetControlsReply, [this](const NetworkPacket::ptr &packet) {
    DriverProtocol::decode(d->controls, packet);
  });
  register_handler(DriverProtocol::SendFrame, [this](const NetworkPacket::ptr &packet) {
    d->image_handler->handle(DriverProtocol::decodeFrame(packet));
  });

  dispatcher->queue_send( DriverProtocol::packetConnectCamera() << DriverProtocol::propertyCameraId(id) );
  wait_for_processed(DriverProtocol::ConnectCameraReply);
  dispatcher->queue_send(DriverProtocol::packetGetCameraName() );
  wait_for_processed(DriverProtocol::GetCameraNameReply);
  dispatcher->queue_send(DriverProtocol::packetGetProperties() );
  wait_for_processed(DriverProtocol::GetPropertiesReply);
}

#define LOG_TO_IMPLEMENT qDebug() << "***** TODO: implement " << __PRETTY_FUNCTION__;
RemoteImager::~RemoteImager()
{
  LOG_TO_IMPLEMENT
}


void RemoteImager::startLive()
{
  dispatcher()->queue_send(DriverProtocol::packetStartLive());
  wait_for_processed(DriverProtocol::StartLiveReply);
}

void RemoteImager::clearROI()
{
  dispatcher()->queue_send(DriverProtocol::packetClearROI());
}

Imager::Controls RemoteImager::controls() const {
  dispatcher()->queue_send(DriverProtocol::packetGetControls() );
  wait_for_processed(DriverProtocol::GetControlsReply);
  return d->controls;
}

QString RemoteImager::name() const
{
  return d->name;
}

Imager::Properties RemoteImager::properties() const
{
  return d->properties;
}

void RemoteImager::setControl(const Imager::Control& control)
{
  LOG_TO_IMPLEMENT
}

void RemoteImager::setROI(const QRect&)
{
  LOG_TO_IMPLEMENT
}



#include "remoteimager.moc"
