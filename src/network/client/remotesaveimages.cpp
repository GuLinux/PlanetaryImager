/*
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

#include "remotesaveimages.h"
#include "network/protocol/savefileprotocol.h"

using namespace std;

DPTR_IMPL(RemoteSaveImages) {
  RemoteSaveImages *q;
};

RemoteSaveImages::RemoteSaveImages(const NetworkDispatcher::ptr& dispatcher) : NetworkReceiver{dispatcher}, dptr(this)
{
  register_handler(SaveFileProtocol::signalSaveFPS, [this](const NetworkPacket::ptr &p) { emit saveFPS(p->payloadVariant().toDouble()); });
  register_handler(SaveFileProtocol::signalMeanFPS, [this](const NetworkPacket::ptr &p) { emit meanFPS(p->payloadVariant().toDouble()); });
  register_handler(SaveFileProtocol::signalSavedFrames, [this](const NetworkPacket::ptr &p) { emit savedFrames(p->payloadVariant().toLongLong()); });
  register_handler(SaveFileProtocol::signalDroppedFrames, [this](const NetworkPacket::ptr &p) { emit droppedFrames(p->payloadVariant().toLongLong()); });
  register_handler(SaveFileProtocol::signalRecording, [this](const NetworkPacket::ptr &p) { emit recording(p->payloadVariant().toString()); });
  register_handler(SaveFileProtocol::signalFinished, [this](const NetworkPacket::ptr &) { emit finished(); });
  
}

RemoteSaveImages::~RemoteSaveImages()
{
}

void RemoteSaveImages::startRecording(Imager* imager)
{
  dispatcher()->queue_send(SaveFileProtocol::packetStartRecording());
}

void RemoteSaveImages::endRecording()
{
  dispatcher()->queue_send(SaveFileProtocol::packetEndRecording());
}

void RemoteSaveImages::handle(const Frame::ptr& frame)
{
}

void RemoteSaveImages::setPaused(bool paused)
{
  dispatcher()->queue_send(SaveFileProtocol::setPaused(paused));
}


#include "remotesaveimages.moc"
