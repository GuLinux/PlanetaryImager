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
#include "commons/utils.h"
#include "commons/fps_counter.h"
#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include "stlutils.h"
#include "Qt/benchmark.h"
#include "Qt/strings.h"
#include "imagerexception.h"
#include <QElapsedTimer>
#include "planetaryimager_mainwindow.h"

using namespace std;

DPTR_IMPL(ImagerThread) : public QObject {
  Q_OBJECT
public:
  Private(const ImagerThread::Worker::ptr& worker, Imager* imager, const ImageHandler::ptr& imageHandler);
  Worker::ptr worker;
  Imager *imager;
  ImageHandler::ptr imageHandler;
  fps_counter fps;
  atomic_bool running;  
  QThread thread;
  boost::lockfree::spsc_queue<Job> jobs_queue;
  void thread_started();
  
  LOG_C_SCOPE(ImagerThread);
};

ImagerThread::Private::Private(const ImagerThread::Worker::ptr& worker, Imager* imager, const ImageHandler::ptr& imageHandler)
  : worker{worker},
  imager{imager},
  imageHandler{imageHandler},
  fps{[=](double rate){ emit imager->fps(rate);}, fps_counter::Mode::Elapsed},
  running{false},
  jobs_queue{20}
{
  connect(&thread, &QThread::started, this, &Private::thread_started);
  moveToThread(&thread);
}


ImagerThread::ImagerThread(const ImagerThread::Worker::ptr& worker, Imager* imager, const ImageHandler::ptr& imageHandler)
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
  QElapsedTimer last_error_occured;
  int error_messages_since_last_success = 0;
  running = true;
  while(running) {
    Job queued_job;
    while(jobs_queue.pop(queued_job)) {
      if(queued_job) {
        try {
          queued_job();
        } catch(const std::exception &e) {
          PlanetaryImagerMainWindow::queue_notify(PlanetaryImagerMainWindow::Warning, tr("Error on imager task"), tr("An error occured during an imager operation for %1:\n%2") 
            % imager->name()
            % e.what());
          qWarning() << e.what();
        }
      }
    }
    try {
      if(auto frame = worker->shoot()) {
          imageHandler->handle(frame);
        ++fps;
        error_messages_since_last_success = 0;
      }
    } catch(const std::exception &e) {
      qWarning() << e.what();
      if( (last_error_occured.elapsed() > 3000 || ! last_error_occured.isValid()) && error_messages_since_last_success++ < 4) {
        if(error_messages_since_last_success == 4) {
          PlanetaryImagerMainWindow::queue_notify(PlanetaryImagerMainWindow::Warning, tr("Error on frame capture"), tr("An error occured while capturing frame for %1:\n%2\nFollowing errors will be quietly ignored, check the console log for more details.") 
            % imager->name()
            % e.what());
        } else {
          PlanetaryImagerMainWindow::queue_notify(PlanetaryImagerMainWindow::Warning, tr("Error on frame capture"), tr("An error occured while capturing frame for %1:\n%2") 
            % imager->name()
            % e.what());
        }
        last_error_occured.restart();
      }
    }
  }
}

void ImagerThread::push_job(const Job& job)
{
  d->jobs_queue.push(job);
}


#include "imagerthread.moc"
