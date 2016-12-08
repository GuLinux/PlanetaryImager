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

#ifndef SAVEFILEFORWARDER_H
#define SAVEFILEFORWARDER_H

#include "network/networkdispatcher.h"
#include "c++/dptr.h"
#include "image_handlers/saveimages.h"

class SaveFileForwarder : public NetworkReceiver
{
public:
  typedef std::shared_ptr<SaveFileForwarder> ptr;
  SaveFileForwarder(const SaveImages::ptr &save_images, const NetworkDispatcher::ptr &dispatcher);
  ~SaveFileForwarder();
  void setImager(Imager *imager);
private:
  DPTR
};

#endif // SAVEFILEFORWARDER_H
