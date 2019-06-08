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

#include "network/client/networkclient.h"
#include <QtNetwork/QTcpSocket>
#include "network/networkpacket.h"
#include "network/networkdispatcher.h"
#include "protocol/protocol.h"
#include "protocol/driverprotocol.h"
#include "Qt/qt_functional.h"

using namespace std;

DPTR_IMPL(NetworkClient) {
  NetworkDispatcherPtr dispatcher;
  unique_ptr<QTcpSocket> socket;
  bool imager_is_running = false;
  NetworkPacketPtr helloPacket;
};

NetworkClient::NetworkClient(const NetworkDispatcherPtr &dispatcher, QObject *parent)
  : QObject{parent}, NetworkReceiver{dispatcher}, dptr(dispatcher, make_unique<QTcpSocket>())
{
  static bool metatypes_registered = false;
  if(!metatypes_registered) {
    metatypes_registered = true;
    qRegisterMetaType<NetworkClient::Status>("NetworkClient::Status");
  }
  d->dispatcher->setSocket(d->socket.get());
  connect(d->socket.get(), &QTcpSocket::connected, [this]{
    d->dispatcher->send( d->helloPacket );
    DriverProtocol::setFormatParameters(NetworkProtocol::decodeHello(d->helloPacket));
    d->helloPacket.reset();
    wait_for_processed(NetworkProtocol::HelloReply);
    emit connected();
  });
  connect(d->socket.get(), F_PTR(QTcpSocket, error, QTcpSocket::SocketError), [this] {
    emit error(d->socket->errorString());
  });
  connect(d->socket.get(), &QTcpSocket::stateChanged, [this] (QTcpSocket::SocketState s) {
    static QHash<QTcpSocket::SocketState, Status> states { 
      {QTcpSocket::ConnectedState, Connected},
      {QTcpSocket::UnconnectedState, Disconnected},
      {QTcpSocket::ConnectingState, Connecting},
      {QTcpSocket::ClosingState, Disconnected},
    };
    emit statusChanged(states.value(s, Error));
  });
  d->socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
} 

NetworkClient::~NetworkClient()
{
}

void NetworkClient::connectToHost(const QString& host, int port, const NetworkProtocol::FormatParameters &parameters)
{
  d->helloPacket = NetworkProtocol::hello(parameters);
  d->socket->connectToHost(host, port, QTcpSocket::ReadWrite);
  QTimer::singleShot(30000, [=]{
    if(d->socket->state() == QAbstractSocket::ConnectingState) {
      d->socket->close();
      emit error(tr("Connection timeout"));
    }
  });
}

void NetworkClient::disconnectFromHost()
{
  d->socket->close();
}
