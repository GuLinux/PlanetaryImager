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
#include "Qt/functional.h"

using namespace std;
using namespace std::placeholders;

#define DECLARE_HANDLER(name) void name(const NetworkPacket::ptr &p);
DPTR_IMPL(DriverForwarder) {
  PlanetaryImager::ptr planetaryImager;
  DriverForwarder *q;
  Driver::Cameras cameras;
  DECLARE_HANDLER(CameraList)
  DECLARE_HANDLER(ConnectCamera)
  DECLARE_HANDLER(GetCameraName)
  DECLARE_HANDLER(GetProperties)
  DECLARE_HANDLER(StartLive)
  DECLARE_HANDLER(ClearROI)
  DECLARE_HANDLER(SetROI)
  DECLARE_HANDLER(GetControls)
  DECLARE_HANDLER(SetControl)
  DECLARE_HANDLER(CloseCamera)
  void sendFPS(double fps);
  void sendTemperature(double temperature);
  void sendDisconnected();
  void sendControlChanged(const Imager::Control &control);
};

#define REGISTER_HANDLER(protocol, name) register_handler(protocol::name, bind(&Private::name, d.get(), _1));

DriverForwarder::DriverForwarder(const NetworkDispatcher::ptr &dispatcher, const PlanetaryImager::ptr &planetaryImager) 
  : NetworkReceiver{dispatcher}, dptr(planetaryImager, this)
{
  REGISTER_HANDLER(DriverProtocol, CameraList)
  REGISTER_HANDLER(DriverProtocol, ConnectCamera)
  REGISTER_HANDLER(DriverProtocol, GetCameraName)
  REGISTER_HANDLER(DriverProtocol, GetProperties)
  REGISTER_HANDLER(DriverProtocol, StartLive)
  REGISTER_HANDLER(DriverProtocol, ClearROI)
  REGISTER_HANDLER(DriverProtocol, SetROI)
  REGISTER_HANDLER(DriverProtocol, GetControls)
  REGISTER_HANDLER(DriverProtocol, SetControl)
  REGISTER_HANDLER(DriverProtocol, CloseCamera)
  QObject::connect(planetaryImager.get(), &PlanetaryImager::camerasChanged, dispatcher.get(), [this] {
    d->cameras = d->planetaryImager->cameras();
    this->dispatcher()->send(DriverProtocol::sendCameraListReply(d->cameras));
  });
  QObject::connect(planetaryImager.get(), &PlanetaryImager::cameraConnected, dispatcher.get(), [this] {
    this->dispatcher()->send(DriverProtocol::packetConnectCameraReply()); // TODO: add status
    QObject::connect(d->planetaryImager->imager(), &Imager::fps, this->dispatcher().get(), bind(&Private::sendFPS, d.get(), _1));
    QObject::connect(d->planetaryImager->imager(), &Imager::temperature, this->dispatcher().get(), bind(&Private::sendTemperature, d.get(), _1));
    QObject::connect(d->planetaryImager->imager(), &Imager::disconnected, this->dispatcher().get(), bind(&Private::sendDisconnected, d.get()));
    QObject::connect(d->planetaryImager->imager(), &Imager::changed, this->dispatcher().get(), bind(&Private::sendControlChanged, d.get(), _1));
  });
}

DriverForwarder::~DriverForwarder()
{
}

void DriverForwarder::Private::CameraList(const NetworkPacket::ptr& p)
{
  planetaryImager->scanCameras();
}

void DriverForwarder::Private::ConnectCamera(const NetworkPacket::ptr& p)
{
  auto address = reinterpret_cast<Driver::Camera *>(p->payloadVariant().toLongLong());
  auto camera = find_if(begin(cameras), end(cameras), [address](const Driver::Camera::ptr &p){ return p.get() == address; });
  if(camera != cameras.end()) {
    planetaryImager->open(*camera);
  }
}

void DriverForwarder::Private::GetCameraName(const NetworkPacket::ptr& p)
{
  q->dispatcher()->send(DriverProtocol::packetGetCameraNameReply() << planetaryImager->imager()->name().toLatin1());
}

void DriverForwarder::Private::CloseCamera(const NetworkPacket::ptr& p)
{
  planetaryImager->closeImager();
}


void DriverForwarder::Private::ClearROI(const NetworkPacket::ptr& p)
{
  planetaryImager->imager()->clearROI();
}


void DriverForwarder::Private::GetProperties(const NetworkPacket::ptr& p)
{
  q->dispatcher()->send( DriverProtocol::sendGetPropertiesReply(planetaryImager->imager()->properties() ) );
}

void DriverForwarder::Private::StartLive(const NetworkPacket::ptr& p)
{
  planetaryImager->imager()->startLive();
  q->dispatcher()->send( DriverProtocol::packetStartLiveReply() );
}

void DriverForwarder::Private::GetControls(const NetworkPacket::ptr& p)
{
  q->dispatcher()->send(DriverProtocol::sendGetControlsReply(planetaryImager->imager()->controls()));
}

void DriverForwarder::Private::SetControl(const NetworkPacket::ptr& p)
{
  planetaryImager->imager()->setControl(DriverProtocol::decodeControl(p));
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
}

void DriverForwarder::Private::sendControlChanged(const Imager::Control &control)
{
  q->dispatcher()->send( DriverProtocol::controlChanged(control));
}

void DriverForwarder::getStatus(QVariantMap& status)
{
  DriverProtocol::encodeStatus({static_cast<bool>(d->planetaryImager->imager())}, status);
}

void DriverForwarder::Private::SetROI(const NetworkPacket::ptr& p)
{
  QRect roi = p->payloadVariant().toRect();
  planetaryImager->imager()->setROI(roi);
}

