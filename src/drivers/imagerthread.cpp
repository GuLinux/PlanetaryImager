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

#include "imagerthread.h"
#include <QObject>
#include <QThread>
#include "imager.h"
#include "utils.h"
#include "fps_counter.h"
#include <atomic>

using namespace std;

class ImagerThread::Private : public QObject {
  Q_OBJECT
public:
  Private(const ImagerThread::Worker::ptr& worker, Imager* imager, const ImageHandlerPtr& imageHandler);
  Worker::ptr worker;
  Imager *imager;
  ImageHandlerPtr imageHandler;
  LogScope log_current_class;
  fps_counter fps;
  atomic_bool running;  
  QThread thread;
private slots:
  void thread_started();
};

ImagerThread::Private::Private(const ImagerThread::Worker::ptr& worker, Imager* imager, const ImageHandlerPtr& imageHandler)
  : worker{worker},
  imager{imager},
  imageHandler{imageHandler},
  LOG_C_SCOPE,
  fps{[=](double rate){ emit imager->fps(rate);}, fps_counter::Mode::Elapsed},
  running{false}
{
  connect(&thread, &QThread::started, this, &Private::thread_started);
  moveToThread(&thread);
}


ImagerThread::ImagerThread(const ImagerThread::Worker::ptr& worker, Imager* imager, const ImageHandlerPtr& imageHandler)
  : dptr(worker, imager, imageHandler)
{
}

ImagerThread::~ImagerThread()
{
  stop();
}

void ImagerThread::start()
{
  d->thread.start();
}

void ImagerThread::stop()
{
  d->running = false;
  d->thread.quit();
  d->thread.wait();
}

void ImagerThread::Private::thread_started()
{
  running = true;
  worker->start();
  while(running) {
    if(worker->shoot(imageHandler))
      ++fps;
  }
  worker->stop();
}

#include "imagerthread.moc"