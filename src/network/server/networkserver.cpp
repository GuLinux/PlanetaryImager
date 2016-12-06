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

#include "networkserver.h"
#include <QtNetwork/QTcpServer>
#include "network/server/driverforwarder.h"
#include "network/networkdispatcher.h"

using namespace std;

DPTR_IMPL(NetworkServer) {
  NetworkServer *q;
  Driver::ptr driver;
  ImageHandler::ptr handler;
  NetworkDispatcher::ptr dispatcher;
  QTcpServer server;
  DriverForwarder::ptr forwarder;
  void new_connection();
};

NetworkServer::NetworkServer(const Driver::ptr &driver, const ImageHandler::ptr &handler, const NetworkDispatcher::ptr &dispatcher, QObject* parent) : QObject{parent}, dptr(this, driver, handler, dispatcher)
{
  d->server.setMaxPendingConnections(1);
  connect(&d->server, &QTcpServer::newConnection, bind(&Private::new_connection, d.get()));
  d->forwarder = make_shared<DriverForwarder>(dispatcher, driver, handler);
}


NetworkServer::~NetworkServer()
{
}

void NetworkServer::listen(const QString& address, int port)
{
  d->server.listen(QHostAddress{address}, port);
}

void NetworkServer::Private::new_connection()
{
  auto socket = server.nextPendingConnection();
  QObject::connect(socket, &QTcpSocket::disconnected, q, [this, socket] {
    qDebug() << "Client disconnected";
    dispatcher->setSocket(nullptr);
  });
  dispatcher->setSocket(socket);
}




#include "networkserver.moc"
