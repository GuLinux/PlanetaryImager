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

#include <functional>
#include <QObject>
#include "driverforwarder.h"
#include "network/protocol/driverprotocol.h"

using namespace std;
using namespace std::placeholders;

#define DECLARE_HANDLER(name) void name(const NetworkPacket::ptr &p);
DPTR_IMPL(DriverForwarder) {
  Driver::ptr driver;
  ImageHandler::ptr handler;
  OnImagerChanged on_imager_changed;
  DriverForwarder *q;
  Driver::Cameras cameras;
  Imager *imager = nullptr;
  DECLARE_HANDLER(CameraList)
  DECLARE_HANDLER(ConnectCamera)
  DECLARE_HANDLER(GetCameraName)
  DECLARE_HANDLER(GetProperties)
  DECLARE_HANDLER(StartLive)
  DECLARE_HANDLER(ClearROI)
  DECLARE_HANDLER(GetControls)
  DECLARE_HANDLER(SetControl)
  void sendFPS(double fps);
  void sendTemperature(double temperature);
  void sendDisconnected();
  void sendControlChanged(const Imager::Control &control);
};

#define REGISTER_HANDLER(protocol, name) register_handler(protocol::name, bind(&Private::name, d.get(), _1));

DriverForwarder::DriverForwarder(const NetworkDispatcher::ptr &dispatcher, const Driver::ptr& driver, const ImageHandler::ptr &handler, OnImagerChanged on_imager_changed) 
  : NetworkReceiver{dispatcher}, dptr(driver, handler, on_imager_changed, this)
{
  REGISTER_HANDLER(DriverProtocol, CameraList)
  REGISTER_HANDLER(DriverProtocol, ConnectCamera)
  REGISTER_HANDLER(DriverProtocol, GetCameraName)
  REGISTER_HANDLER(DriverProtocol, GetProperties)
  REGISTER_HANDLER(DriverProtocol, StartLive)
  REGISTER_HANDLER(DriverProtocol, ClearROI)
  REGISTER_HANDLER(DriverProtocol, GetControls)
  REGISTER_HANDLER(DriverProtocol, SetControl)
}

DriverForwarder::~DriverForwarder()
{
}

void DriverForwarder::Private::CameraList(const NetworkPacket::ptr& p)
{
      cameras = driver->cameras();
      q->dispatcher()->send(DriverProtocol::sendCameraListReply(cameras));
}

void DriverForwarder::Private::ConnectCamera(const NetworkPacket::ptr& p)
{
  delete imager;
  imager = nullptr;
  auto address = reinterpret_cast<Driver::Camera *>(p->payloadVariant().toLongLong());
  if(count_if(begin(cameras), end(cameras), [address](const Driver::Camera::ptr &p){ return p.get() == address; }) == 1) {
    imager = address->imager(handler);
  }
  if(on_imager_changed)
    on_imager_changed(imager);
  q->dispatcher()->send(DriverProtocol::packetConnectCameraReply()); // TODO: add status
  QObject::connect(imager, &Imager::fps, q->dispatcher().get(), bind(&Private::sendFPS, this, _1));
  QObject::connect(imager, &Imager::temperature, q->dispatcher().get(), bind(&Private::sendTemperature, this, _1));
  QObject::connect(imager, &Imager::disconnected, q->dispatcher().get(), bind(&Private::sendDisconnected, this));
  QObject::connect(imager, &Imager::changed, q->dispatcher().get(), bind(&Private::sendControlChanged, this, _1), Qt::QueuedConnection);
}

void DriverForwarder::Private::GetCameraName(const NetworkPacket::ptr& p)
{
  q->dispatcher()->send(DriverProtocol::packetGetCameraNameReply() << imager->name().toLatin1());
}

void DriverForwarder::Private::ClearROI(const NetworkPacket::ptr& p)
{
  imager->clearROI();
}


void DriverForwarder::Private::GetProperties(const NetworkPacket::ptr& p)
{
  q->dispatcher()->send( DriverProtocol::sendGetPropertiesReply(imager->properties() ) );
}

void DriverForwarder::Private::StartLive(const NetworkPacket::ptr& p)
{
  imager->startLive();
  q->dispatcher()->send( DriverProtocol::packetStartLiveReply() );
}

void DriverForwarder::Private::GetControls(const NetworkPacket::ptr& p)
{
  q->dispatcher()->send(DriverProtocol::sendGetControlsReply(imager->controls()));
}

void DriverForwarder::Private::SetControl(const NetworkPacket::ptr& p)
{
  imager->setControl(DriverProtocol::decodeControl(p));
}


void DriverForwarder::Private::sendFPS(double fps)
{
  q->dispatcher()->send( DriverProtocol::packetsignalFPS() << QVariant{fps});
}

void DriverForwarder::Private::sendTemperature(double temperature)
{
  q->dispatcher()->send( DriverProtocol::packetsignalTemperature() << QVariant{temperature});
}

void DriverForwarder::Private::sendDisconnected()
{
  q->dispatcher()->send( DriverProtocol::packetsignalDisconnected());
  imager->deleteLater();
  imager = nullptr;
}

void DriverForwarder::Private::sendControlChanged(const Imager::Control &control)
{
  q->dispatcher()->send( DriverProtocol::controlChanged(control));
}

void DriverForwarder::getStatus(QVariantMap& status)
{
  DriverProtocol::encodeStatus({static_cast<bool>(d->imager)}, status);
}

