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
#include "network/networkdispatcher.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include "network/networkpacket.h"

using namespace std;

DPTR_IMPL(RemoteImager) {
  const ImageHandlerPtr image_handler;
  QString name;
  Properties properties;
  Controls controls;
  bool live_was_started = true;
};

RemoteImager::RemoteImager(const ImageHandlerPtr& image_handler, const NetworkDispatcherPtr &dispatcher, qlonglong id) : Imager{image_handler}, NetworkReceiver{dispatcher}, dptr(image_handler)
{
  register_handler(DriverProtocol::signalCameraConnected, [](const NetworkPacketPtr &) {});
  register_handler(DriverProtocol::GetCameraNameReply, [this](const NetworkPacketPtr &packet) {
    d->name = packet->payloadVariant().toString();
  });
  register_handler(DriverProtocol::GetPropertiesReply, [this](const NetworkPacketPtr &packet) {
    DriverProtocol::decode(d->properties, packet);
  });
  register_handler(DriverProtocol::GetControlsReply, [this](const NetworkPacketPtr &packet) {
    DriverProtocol::decode(d->controls, packet);
  });
  register_handler(DriverProtocol::SendFrame, [this](const NetworkPacketPtr &packet) {
    //qDebug() << "Got frame";
    QtConcurrent::run([=] {
      auto frame = DriverProtocol::decodeFrame(packet);
      d->image_handler->handle(frame);
    });
  });
  register_handler(DriverProtocol::signalFPS, [this](const NetworkPacketPtr &packet) {
    emit fps(packet->payloadVariant().toDouble());
  });
  register_handler(DriverProtocol::signalTemperature, [this](const NetworkPacketPtr &packet) {
    emit temperature(packet->payloadVariant().toDouble());
  });
  register_handler(DriverProtocol::signalDisconnected, [this](const NetworkPacketPtr &) { emit disconnected(); });
  register_handler(DriverProtocol::signalControlChanged, [this](const NetworkPacketPtr &packet) {
    auto control = DriverProtocol::decodeControl(packet) ;
    emit changed( control );
  });

  if(id != -1) {
    dispatcher->queue_send( DriverProtocol::packetConnectCamera() << id );
    wait_for_processed(DriverProtocol::signalCameraConnected);
    d->live_was_started = false;
  }
  dispatcher->queue_send(DriverProtocol::packetGetCameraName() );
  wait_for_processed(DriverProtocol::GetCameraNameReply);
  dispatcher->queue_send(DriverProtocol::packetGetProperties() );
  wait_for_processed(DriverProtocol::GetPropertiesReply);
}

#define LOG_TO_IMPLEMENT qDebug() << "***** TODO: implement " << __PRETTY_FUNCTION__;


RemoteImager::~RemoteImager()
{
  dispatcher()->queue_send(DriverProtocol::packetCloseCamera());
}


void RemoteImager::startLive()
{
  if(!d->live_was_started) { // TODO: fix the liveStarted: should be read from camera instead, and agreed in mainWindow
    dispatcher()->queue_send(DriverProtocol::packetStartLive());
    wait_for_processed(DriverProtocol::StartLiveReply);
  }
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
  dispatcher()->queue_send(DriverProtocol::setControl(control) );
}

void RemoteImager::setROI(const QRect &roi)
{
  dispatcher()->queue_send(DriverProtocol::packetSetROI() << roi);
}



#include "remoteimager.moc"
