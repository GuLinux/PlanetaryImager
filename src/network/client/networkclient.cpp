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

#include "networkclient.h"
#include <QtNetwork/QTcpSocket>
#include "network/networkdispatcher.h"
#include "protocol/protocol.h"
using namespace std;

DPTR_IMPL(NetworkClient) {
  NetworkDispatcher::ptr dispatcher;
  QTcpSocket socket;
};

NetworkClient::NetworkClient(const NetworkDispatcher::ptr &dispatcher, QObject *parent) : QObject{parent}, NetworkReceiver{dispatcher}, dptr(dispatcher)
{
  d->dispatcher->setSocket(&d->socket);
  register_handler(NetworkProtocol::HelloReply, [this](const NetworkPacket::ptr &p) {});
}

NetworkClient::~NetworkClient()
{
}

void NetworkClient::connectToHost(const QString& host, int port)
{
  connect(&d->socket, &QTcpSocket::connected, [this]{
    d->dispatcher->send(NetworkProtocol::packetHello() );
    wait_for_processed(NetworkProtocol::HelloReply);
    emit connected();
  });
  d->socket.connectToHost(host, port, QTcpSocket::ReadWrite);
}


#include "networkclient.moc"
