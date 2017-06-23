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

#ifndef REMOTESAVEIMAGES_H
#define REMOTESAVEIMAGES_H

#include "network/networkdispatcher.h"
#include "image_handlers/saveimages.h"
#include "c++/dptr.h"

class RemoteSaveImages : public SaveImages, public NetworkReceiver
{
Q_OBJECT
public:
  RemoteSaveImages(const NetworkDispatcher::ptr &dispatcher);
  ~RemoteSaveImages();

public slots:
  void startRecording(Imager * imager) override;
  void endRecording() override;
  void setPaused(bool paused) override;
private:

  void doHandle(Frame::ptr frame) override { }

  DPTR
};

#endif // REMOTESAVEIMAGES_H
