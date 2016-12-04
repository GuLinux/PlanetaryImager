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
#include "driverforwarder.h"
#include "network/protocol/driverprotocol.h"
using namespace std;
using namespace std::placeholders;

#define DECLARE_HANDLER(name) void name(const NetworkPacket::ptr &p);
DPTR_IMPL(DriverForwarder) {
  Driver::ptr driver;
  ImageHandler::ptr handler;
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
};

#define REGISTER_HANDLER(protocol, name) register_handler(protocol::name, bind(&Private::name, d.get(), _1));

DriverForwarder::DriverForwarder(const NetworkDispatcher::ptr &dispatcher, const Driver::ptr& driver, const ImageHandler::ptr &handler) : NetworkReceiver{dispatcher}, dptr(driver, handler, this)
{
  REGISTER_HANDLER(DriverProtocol, CameraList)
  REGISTER_HANDLER(DriverProtocol, ConnectCamera)
  REGISTER_HANDLER(DriverProtocol, GetCameraName)
  REGISTER_HANDLER(DriverProtocol, GetProperties)
  REGISTER_HANDLER(DriverProtocol, StartLive)
  REGISTER_HANDLER(DriverProtocol, ClearROI)
  REGISTER_HANDLER(DriverProtocol, GetControls)
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
    auto address = reinterpret_cast<Driver::Camera *>(p->property(DriverProtocol::CameraId).toLongLong());
    if(count_if(begin(cameras), end(cameras), [address](const Driver::Camera::ptr &p){ return p.get() == address; }) == 1) {
      imager = address->imager(handler);
    }
    q->dispatcher()->send(DriverProtocol::packetConnectCameraReply()); // TODO: add status
}

void DriverForwarder::Private::GetCameraName(const NetworkPacket::ptr& p)
{
  q->dispatcher()->send(DriverProtocol::packetGetCameraNameReply() << DriverProtocol::propertyCameraName(imager->name()));
}

void DriverForwarder::Private::ClearROI(const NetworkPacket::ptr& p)
{
  imager->clearROI();
}


void DriverForwarder::Private::GetProperties(const NetworkPacket::ptr& p)
{
  imager->properties();
  q->dispatcher()->send( DriverProtocol::sendGetPropertiesReply(imager->properties() ) );
}

void DriverForwarder::Private::StartLive(const NetworkPacket::ptr& p)
{
  imager->startLive();
}

void DriverForwarder::Private::GetControls(const NetworkPacket::ptr& p)
{
  q->dispatcher()->send(DriverProtocol::sendGetControlsReply(imager->controls()));
}
