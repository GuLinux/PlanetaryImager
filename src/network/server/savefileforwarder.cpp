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

#include "savefileforwarder.h"
#include <QObject>
#include "network/protocol/savefileprotocol.h"
#include "image_handlers/saveimages.h"
#include "network/networkdispatcher.h"
#include "network/networkpacket.h"

DPTR_IMPL(SaveFileForwarder) {
  const SaveImagesPtr save_images;
  SaveFileForwarder *q;
  Imager *imager = nullptr;
};

SaveFileForwarder::SaveFileForwarder(const SaveImagesPtr& save_images, const NetworkDispatcherPtr& dispatcher) : NetworkReceiver{dispatcher}, dptr(save_images)
{
  register_handler(SaveFileProtocol::StartRecording, [this](const NetworkPacketPtr &) { d->save_images->startRecording(d->imager); });
  register_handler(SaveFileProtocol::slotSetPaused, [this](const NetworkPacketPtr &p) { d->save_images->setPaused(p->payloadVariant().toBool()); });
  register_handler(SaveFileProtocol::EndRecording, [this](const NetworkPacketPtr &) { d->save_images->endRecording(); });
  QObject::connect(save_images.get(), &SaveImages::saveFPS, save_images.get(), [this](double fps) { this->dispatcher()->queue_send(SaveFileProtocol::packetsignalSaveFPS() << QVariant{fps}); } );
  QObject::connect(save_images.get(), &SaveImages::meanFPS, save_images.get(), [this](double fps) { this->dispatcher()->queue_send(SaveFileProtocol::packetsignalMeanFPS() << QVariant{fps}); } );
  QObject::connect(save_images.get(), &SaveImages::savedFrames, save_images.get(), [this](long frames) { this->dispatcher()->queue_send(SaveFileProtocol::packetsignalSavedFrames() << QVariant{static_cast<qlonglong>(frames)}); } );
  QObject::connect(save_images.get(), &SaveImages::droppedFrames, save_images.get(), [this](long frames) { this->dispatcher()->queue_send(SaveFileProtocol::packetsignalDroppedFrames() << QVariant{static_cast<qlonglong>(frames)}); } );
  QObject::connect(save_images.get(), &SaveImages::recording, save_images.get(), [this](const QString &file) {
    emit isRecording(true);
    this->dispatcher()->queue_send(SaveFileProtocol::packetsignalRecording() << QVariant{file});
  } );
  QObject::connect(save_images.get(), &SaveImages::finished, save_images.get(), [this]{
    this->dispatcher()->queue_send(SaveFileProtocol::packetsignalFinished());
    emit isRecording(false);
  } );
}

void SaveFileForwarder::setImager(Imager* imager)
{
  d->imager = imager;
}


SaveFileForwarder::~SaveFileForwarder()
{
}
