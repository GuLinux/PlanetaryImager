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
#include "image_handlers/imagehandler.h"
#include <QObject>
#include <QThread>
#include "imager.h"
#include "commons/utils.h"
#include "commons/fps_counter.h"
#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include "stlutils.h"
#include "Qt/benchmark.h"
#include "Qt/qt_strings_helper.h"
#include "imagerexception.h"
#include <QElapsedTimer>
#include "commons/messageslogger.h"
#include "commons/frame.h"


using namespace std;
using namespace std::chrono_literals;

DPTR_IMPL(ImagerThread) : public QObject {
  Q_OBJECT
public:
  Private(const ImagerThread::Worker::ptr& worker, Imager* imager, const ImageHandlerPtr& imageHandler);
  Worker::ptr worker;
  Imager *imager;
  ImageHandlerPtr imageHandler;
  fps_counter fps;
  atomic_bool running;
  QThread thread;
  boost::lockfree::spsc_queue<Job> jobs_queue;
  bool long_exposure_mode = false;
  chrono::duration<double> exposure;
  Configuration::CaptureEndianess captureEndianess = Configuration::CaptureEndianess::CameraDefault;

  void thread_started();

  LOG_C_SCOPE(ImagerThread);
};

ImagerThread::Private::Private(const ImagerThread::Worker::ptr& worker, Imager* imager, const ImageHandlerPtr& imageHandler)
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


ImagerThread::ImagerThread(const ImagerThread::Worker::ptr& worker, Imager* imager, const ImageHandlerPtr& imageHandler,
                           Configuration::CaptureEndianess captureEndianess)
  : dptr(worker, imager, imageHandler)
{
    d->captureEndianess = captureEndianess;
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
  QElapsedTimer firstErrorOccured;
  int errors_since_last_success = 0;
  int error_messages_since_last_success = 0;
  running = true;
  while(running) {
    Job queued_job;
    while(jobs_queue.pop(queued_job)) {
      if(queued_job) {
        try {
          queued_job();
        } catch(const std::exception &e) {
          MessagesLogger::queue(MessagesLogger::Warning, tr("Error on imager task"), tr("An error occurred during an imager operation for %1:\n%2")
            % imager->name()
            % e.what());
          qWarning() << e.what();
        }
      }
    }
    try {
      if(long_exposure_mode)
        imager->long_exposure_started(exposure.count() );
      if(auto frame = worker->shoot()) {
          frame->set_exposure(exposure);
          if (captureEndianess != Configuration::CaptureEndianess::CameraDefault)
              frame->overrideByteOrder(captureEndianess == Configuration::CaptureEndianess::Little ? Frame::ByteOrder::LittleEndian
                                                                                                   : Frame::ByteOrder::BigEndian);
          imageHandler->handle(frame);
        ++fps;
        errors_since_last_success = 0;
      }
    } catch(const Imager::exception &e) {
      qWarning() << e.what();
      if(e.imagerDisconnected()) {
        running = false;
        imager->destroy();
        return;
      }
      if(errors_since_last_success++ == 0) {
        error_messages_since_last_success = 0;
        firstErrorOccured.restart();
        continue;
      }
      if(firstErrorOccured.elapsed() > 30 * 1000 && error_messages_since_last_success++ == 0) {
        MessagesLogger::queue(MessagesLogger::Warning, tr("Error on frame capture"), tr("An error occurred while capturing frame for %1:\n%2")
        % imager->name()
        % e.what());
        continue;
      }

      if(firstErrorOccured.elapsed() > 90 * 1000) {
          MessagesLogger::queue(MessagesLogger::Warning, tr("Error on frame capture"), tr("An error occurred while capturing frame for %1:\n%2\nDisconnecting camera.")
            % imager->name()
            % e.what());
          running = false;
          imager->destroy();
          return;
      }
    }
    if(long_exposure_mode)
      imager->long_exposure_ended();
  }
}

shared_ptr<QWaitCondition> ImagerThread::push_job(const Job& job)
{
  auto wait_condition = make_shared<QWaitCondition>();
  auto job_sync = [wait_condition, job]{
    GuLinux::Scope wake{[=]{ wait_condition->wakeAll();}};
    job();
  };
  if(! d->jobs_queue.push(job_sync) )
    return {};
  return wait_condition;
}


void ImagerThread::set_exposure(const std::chrono::duration<double> &exposure)
{
  d->long_exposure_mode = ( exposure >= 2s);
  d->exposure = exposure;
  qDebug() << "Exposure: " << exposure.count() << "s; long exposure: " << d->long_exposure_mode;
}

void ImagerThread::setCaptureEndianess(Configuration::CaptureEndianess captureEndianess)
{
    d->captureEndianess = captureEndianess;
    qDebug() << "Capture endianess changed to " << static_cast<int>(captureEndianess);
}

#include "imagerthread.moc"
