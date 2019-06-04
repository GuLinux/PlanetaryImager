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

#ifndef THREADIMAGEHANDLER_H
#define THREADIMAGEHANDLER_H
#include <QObject>
#include "c++/dptr.h"
#include "image_handlers/imagehandler.h"

class ThreadImageHandler : public ImageHandler
{
public:
  ThreadImageHandler(const  ImageHandlerPtr &imageHandler);
  virtual ~ThreadImageHandler();

private:

  void doHandle(FrameConstPtr frame) override;

  DPTR
};

#endif // THREADIMAGEHANDLER_H
