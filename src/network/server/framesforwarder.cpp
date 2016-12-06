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

#include "framesforwarder.h"
#include <boost/lockfree/spsc_queue.hpp>
#include <QObject>
#include "network/protocol/driverprotocol.h"

using namespace std;

DPTR_IMPL(FramesForwarder) {
  NetworkDispatcher::ptr dispatcher;
  FramesForwarder *q;
  boost::lockfree::spsc_queue<Frame::ptr, boost::lockfree::capacity<3>> queue;
};

FramesForwarder::FramesForwarder(const NetworkDispatcher::ptr& dispatcher) : dptr(dispatcher, this)
{
}

FramesForwarder::~FramesForwarder()
{
}

void FramesForwarder::handle(const Frame::ptr& frame)
{
  if(d->queue.push(frame)) {
    QMetaObject::invokeMethod(this, "send_frames", Qt::QueuedConnection);
  }
}

void FramesForwarder::send_frames()
{
  Frame::ptr frame;
  while(d->queue.pop(frame)) {
    d->dispatcher->send(DriverProtocol::sendFrame(frame));
  }
}

#include "framesforwarder.moc"
