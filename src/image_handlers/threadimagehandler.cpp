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
#include "commons/frame.h"

using namespace std;

DPTR_IMPL(ThreadImageHandler) {
  ThreadImageHandler *q;
  unique_ptr<QThread> thread;
  class Worker;
  unique_ptr<Worker> worker;
};

class ThreadImageHandler::Private::Worker : public QObject {
  Q_OBJECT

public:
  Worker(const ImageHandlerPtr &imageHandler, QObject *parent = nullptr);
public slots:
  void handle(FrameConstPtr frame);
private:
  ImageHandlerPtr imageHandler;
};

ThreadImageHandler::Private::Worker::Worker(const ImageHandlerPtr& imageHandler, QObject* parent)
  : QObject{parent}, imageHandler{imageHandler}
{
}


void ThreadImageHandler::Private::Worker::handle(FrameConstPtr frame)
{
  imageHandler->handle(frame);
}


ThreadImageHandler::ThreadImageHandler(const ImageHandlerPtr &imageHandler) : dptr(this, make_unique<QThread>())
{
  d->worker = make_unique<Private::Worker>(imageHandler);
  d->worker->moveToThread(d->thread.get());
  d->thread->start();
}

ThreadImageHandler::~ThreadImageHandler()
{
  d->thread->quit();
  d->thread->wait();
}

void ThreadImageHandler::doHandle(FrameConstPtr frame)
{
  QMetaObject::invokeMethod(d->worker.get(), "handle", Qt::QueuedConnection, Q_ARG(FrameConstPtr, frame));
}

#include "threadimagehandler.moc"
