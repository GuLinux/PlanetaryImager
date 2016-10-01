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

#ifndef IMAGERTHREAD_H
#define IMAGERTHREAD_H
#include <functional>
#include "image_handlers/imagehandler.h"
#include "dptr.h"
class Imager;
class ImagerThread
{
  public:
  typedef std::shared_ptr<ImagerThread> ptr;
  typedef std::function<void()> Job;
  class Worker {
  public:
    virtual Frame::ptr shoot() = 0;
    typedef std::shared_ptr<Worker> ptr;
    typedef std::function<ptr()> factory;
  };
  ImagerThread(const Worker::ptr& worker, Imager* imager, const ImageHandler::ptr& imageHandler);
  ~ImagerThread();
  void stop();
  void start();
  void push_job(const Job &job);
private:
  DPTR

};

#endif // IMAGERTHREAD_H
