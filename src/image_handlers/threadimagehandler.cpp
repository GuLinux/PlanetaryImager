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

#include "threadimagehandler.h"
#include <QThread>

using namespace std;

DPTR_IMPL(ThreadImageHandler) {
  ThreadImageHandler *q;
  QThread thread;
  class Worker;
  unique_ptr<Worker> worker;
};

class ThreadImageHandler::Private::Worker : public QObject, public ImageHandler {
  Q_OBJECT
public:
  Worker(const ImageHandler::ptr &imageHandler, QObject *parent = nullptr);
public slots:
  virtual void handle(const Frame::ptr &frame);
private:
  ImageHandler::ptr imageHandler;
};

ThreadImageHandler::Private::Worker::Worker(const ImageHandler::ptr& imageHandler, QObject* parent) 
  : QObject{parent}, imageHandler{imageHandler}
{
}


void ThreadImageHandler::Private::Worker::handle(const Frame::ptr& frame)
{
  imageHandler->handle(frame);
}


ThreadImageHandler::ThreadImageHandler(const ImageHandler::ptr &imageHandler) : dptr(this)
{
  d->worker = make_unique<Private::Worker>(imageHandler);
  d->worker->moveToThread(&d->thread);
  d->thread.start();
}

ThreadImageHandler::~ThreadImageHandler()
{
  d->thread.quit();
  d->thread.wait();
}

void ThreadImageHandler::handle(const Frame::ptr& frame)
{
  QMetaObject::invokeMethod(d->worker.get(), "handle", Qt::QueuedConnection, Q_ARG(Frame::ptr, frame));
}

#include "threadimagehandler.moc"
