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

#include "driverprotocol.h"
#include <algorithm>
using namespace std;
const QString DriverProtocol::CameraList = "Driver_CameraList";
const QString DriverProtocol::CameraListReply = "Driver_CameraList_reply";
const QString DriverProtocol::CamerasParameter = "Driver_CameraList_param";
const QString DriverProtocol::ConnectCamera = "Driver_ConnectCamera";

void DriverProtocol::encode(const Driver::Cameras& cameras, const NetworkPacket::ptr& packet)
{
  QVariantList v_cameras;
  transform(begin(cameras), end(cameras), back_inserter(v_cameras), [](const Driver::Camera::ptr &c){
    QVariantMap p;
    p["n"] = c->name();
    p["a"] = reinterpret_cast<long long>(c.get());
    return p;
  });
  packet->setProperty("cameras", v_cameras);
}
namespace {
class RemoteCamera : public Driver::Camera {
public:
  RemoteCamera(const QVariant &data, const NetworkDispatcher::ptr &dispatcher) : data{data.toMap()}, dispatcher{dispatcher} {}
  Imager * imager(const ImageHandler::ptr & imageHandler) const override;
  QString name() const override;
private:
  const QVariantMap data;
  const NetworkDispatcher::ptr dispatcher;
};
}

Imager *RemoteCamera::imager(const ImageHandler::ptr & imageHandler) const {
  return nullptr;
}
QString RemoteCamera::name() const {
  return data["n"].toString();
}

void DriverProtocol::decode(Driver::Cameras& cameras, const NetworkPacket::ptr& packet, const NetworkDispatcher::ptr dispatcher)
{
  auto v_cameras = packet->property("cameras").toList();
  transform(begin(v_cameras), end(v_cameras), back_inserter(cameras), [&](const QVariant &v) { return make_shared<RemoteCamera>(v, dispatcher); });
}


