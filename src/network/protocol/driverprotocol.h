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

#ifndef DRIVERPROTOCOL_H
#define DRIVERPROTOCOL_H
#include "network/protocol/protocol.h"
#include <QList>
#include "commons/fwd.h"
#include "drivers/imager.h"

FWD_PTR(Frame)
FWD_PTR(Camera)
FWD_PTR(NetworkPacket)

class DriverProtocol : public NetworkProtocol {
public:
  ADD_PROTOCOL_PACKET_NAME(CameraList)
  ADD_PROTOCOL_PACKET_NAME(CameraListReply)
  ADD_PROTOCOL_PACKET_NAME(ConnectCamera)
  ADD_PROTOCOL_PACKET_NAME(CloseCamera)
  ADD_PROTOCOL_PACKET_NAME(GetCameraName)
  ADD_PROTOCOL_PACKET_NAME(GetCameraNameReply)
  ADD_PROTOCOL_PACKET_NAME(GetProperties)
  ADD_PROTOCOL_PACKET_NAME(GetPropertiesReply)
  ADD_PROTOCOL_PACKET_NAME(StartLive)
  ADD_PROTOCOL_PACKET_NAME(StartLiveReply)
  ADD_PROTOCOL_PACKET_NAME(ClearROI)
  ADD_PROTOCOL_PACKET_NAME(GetControls)
  ADD_PROTOCOL_PACKET_NAME(GetControlsReply)
  ADD_PROTOCOL_PACKET_NAME(SendFrame)
  ADD_PROTOCOL_PACKET_NAME(SetControl)
  ADD_PROTOCOL_PACKET_NAME(SetROI)

  ADD_PROTOCOL_PACKET_NAME(signalFPS)
  ADD_PROTOCOL_PACKET_NAME(signalTemperature)
  ADD_PROTOCOL_PACKET_NAME(signalControlChanged)
  ADD_PROTOCOL_PACKET_NAME(signalCameraConnected)
  ADD_PROTOCOL_PACKET_NAME(signalDisconnected)

  // TODO: remove all the static methods, particularly since most of them are stateful

  static NetworkPacketPtr sendCameraListReply(const QList<CameraPtr> &cameras);
  typedef std::function<CameraPtr(const QString &, qlonglong)> CameraFactory;
  static void decode(QList<CameraPtr> &cameras, const NetworkPacketPtr &packet, const CameraFactory &factory);

  static NetworkPacketPtr sendGetPropertiesReply(const Imager::Properties &properties);
  static void decode(Imager::Properties &properties, const NetworkPacketPtr &packet);

  static NetworkPacketPtr sendGetControlsReply(const Imager::Controls &controls);
  static void decode(Imager::Controls &controls, const NetworkPacketPtr &packet);

  static NetworkPacketPtr setControl(const Imager::Control &control);
  static NetworkPacketPtr controlChanged(const Imager::Control &control);
  static Imager::Control decodeControl(const NetworkPacketPtr &packet);

  static void setFormatParameters(const FormatParameters &parameters);
  static NetworkPacketPtr sendFrame(FrameConstPtr frame);
  static FramePtr decodeFrame(const NetworkPacketPtr &packet);

  static bool isForwardingEnabled();

  struct DriverStatus {
    bool imager_running;
  };
  static DriverStatus decodeStatus(const NetworkPacketPtr &packet);
  static void encodeStatus(const DriverStatus &status, QVariantMap &data);
};

#endif // DRIVERPROTOCOL_H
