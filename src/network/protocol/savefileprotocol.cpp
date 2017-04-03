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

#include "savefileprotocol.h"

using namespace std;
PROTOCOL_NAME_VALUE(SaveFile, StartRecording);
PROTOCOL_NAME_VALUE(SaveFile, EndRecording);
PROTOCOL_NAME_VALUE(SaveFile, signalSaveFPS);
PROTOCOL_NAME_VALUE(SaveFile, signalMeanFPS);
PROTOCOL_NAME_VALUE(SaveFile, signalSavedFrames);
PROTOCOL_NAME_VALUE(SaveFile, signalDroppedFrames);
PROTOCOL_NAME_VALUE(SaveFile, signalRecording);
PROTOCOL_NAME_VALUE(SaveFile, signalFinished);
PROTOCOL_NAME_VALUE(SaveFile, slotSetPaused);


NetworkPacket::ptr SaveFileProtocol::setPaused(bool paused)
{
  return packetslotSetPaused() << paused;
}
