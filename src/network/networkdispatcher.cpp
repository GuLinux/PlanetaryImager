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

#include "networkdispatcher.h"
#include <QHash>
#include <QHash>
#include <QCoreApplication>
#include "commons/utils.h"

using namespace std;

DPTR_IMPL(NetworkDispatcher) {
  QSet<NetworkReceiver *> receivers;
  QTcpSocket *socket = nullptr;
  void readyRead();
  uint64_t written;
  uint64_t sent;
};


DPTR_IMPL(NetworkReceiver) {
  const NetworkDispatcher::ptr dispatcher;
  QHash<NetworkPacket::Type, bool> packets_processed;
  QHash<NetworkPacket::Type, NetworkReceiver::HandlePacket> handlers;
};

NetworkReceiver::NetworkReceiver(const NetworkDispatcher::ptr &dispatcher) : dptr(dispatcher)
{
  dispatcher->attach(this);
}

NetworkReceiver::~NetworkReceiver()
{
  d->dispatcher->detach(this);
}

NetworkDispatcher::ptr NetworkReceiver::dispatcher() const
{
  return d->dispatcher;
}


void NetworkReceiver::wait_for_processed(const NetworkPacket::Type &name) const
{
  if(! d->dispatcher->is_connected())
    return;
  d->packets_processed[name] = false;
  while(! d->packets_processed[name] && d->dispatcher->is_connected())
    qApp->processEvents();
}



void NetworkReceiver::register_handler(const NetworkPacket::Type& name, const HandlePacket handler)
{
  d->handlers[name] = handler;
}

void NetworkReceiver::handle(const NetworkPacket::ptr& packet)
{
  auto handler = d->handlers[packet->name()];
  if(handler)
    handler(packet);
  d->packets_processed[packet->name()] = true;
}



NetworkDispatcher::NetworkDispatcher(QObject* parent) : QObject{parent}, dptr()
{
  static bool metatypes_registered = false;
  if(!metatypes_registered) {
    metatypes_registered = true;
    qRegisterMetaType<NetworkPacket::ptr>("NetworkPacket::ptr");
  }
}

NetworkDispatcher::~NetworkDispatcher()
{
}

void NetworkDispatcher::attach(NetworkReceiver* receiver)
{
  d->receivers.insert(receiver);
}

void NetworkDispatcher::detach(NetworkReceiver* receiver)
{
  d->receivers.remove(receiver);
}

void NetworkDispatcher::setSocket(QTcpSocket* socket)
{
  //delete d->socket;
  if(d->socket)
    d->socket->disconnect(this, 0);
  d->socket = socket;
  if(! socket)
    return;
  d->written = 0;
  d->sent = 0;
  connect(socket, &QTcpSocket::bytesWritten, this, [=](qint64 written){
    d->sent += written;
    emit bytes(d->written, d->sent);
  });
  connect(socket, &QTcpSocket::readyRead, this, bind(&Private::readyRead, d.get()));
}

void NetworkDispatcher::send(const NetworkPacket::ptr &packet) {
  if(! is_connected())
    return;
  //qDebug() << packet->name();
  auto written = packet->sendTo(d->socket);
  d->written += written;
}

void NetworkDispatcher::queue_send(const NetworkPacket::ptr& packet)
{
  QMetaObject::invokeMethod(this, "send", Q_ARG(NetworkPacket::ptr, packet) );
}


void NetworkDispatcher::Private::readyRead()
{
  QList<NetworkPacket::ptr> packets;
  while(socket->bytesAvailable() > 0) {
    //qDebug() << socket->bytesAvailable();
    auto packet = make_shared<NetworkPacket>();
    packet->receiveFrom(socket);
    packets.push_back(packet);
    //qDebug() << packet->name();
  }
  for(auto packet: packets) {
    for(auto receiver: receivers)
      receiver->handle(packet);
  }
}

bool NetworkDispatcher::is_connected() const
{
  return d->socket && d->socket->isValid() && d->socket->isOpen();
}

