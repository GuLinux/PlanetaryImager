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

#ifndef SAVEFILEPROTOCOL_H
#define SAVEFILEPROTOCOL_H
#include "protocol/protocol.h"
#include "commons/fwd.h"

FWD_PTR(NetworkPacket)

class SaveFileProtocol : public NetworkProtocol
{
public:
  ADD_PROTOCOL_PACKET_NAME(StartRecording)
  ADD_PROTOCOL_PACKET_NAME(EndRecording)
  ADD_PROTOCOL_PACKET_NAME(slotSetPaused)
  ADD_PROTOCOL_PACKET_NAME(signalSaveFPS)
  ADD_PROTOCOL_PACKET_NAME(signalMeanFPS)
  ADD_PROTOCOL_PACKET_NAME(signalSavedFrames)
  ADD_PROTOCOL_PACKET_NAME(signalDroppedFrames)
  ADD_PROTOCOL_PACKET_NAME(signalRecording)
  ADD_PROTOCOL_PACKET_NAME(signalFinished)
  static NetworkPacketPtr setPaused(bool paused);
};

#endif // SAVEFILEPROTOCOL_H
