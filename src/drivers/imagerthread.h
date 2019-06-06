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

#include <commons/configuration.h>
#include <functional>
#include "dptr.h"
#include <chrono>
#include "commons/fwd.h"
FWD_PTR(Frame)
FWD(Imager)
FWD_PTR(ImagerThread)
FWD_PTR(QWaitCondition)
FWD_PTR(ImageHandler)

class ImagerThread
{
  public:
  typedef std::function<void()> Job;
  class Worker {
  public:
    virtual FramePtr shoot() = 0;
    typedef std::shared_ptr<Worker> ptr;
    typedef std::function<ptr()> factory;
  };
  ImagerThread(const Worker::ptr& worker, Imager* imager, const ImageHandlerPtr& imageHandler, Configuration::CaptureEndianess captureEndianess);
  ~ImagerThread();
  void stop();
  void start();
  QWaitConditionPtr push_job(const Job &job);
  void set_exposure(const std::chrono::duration<double> &exposure);
  void setCaptureEndianess(Configuration::CaptureEndianess captureEndianess);
private:
  DPTR

};

#endif // IMAGERTHREAD_H
