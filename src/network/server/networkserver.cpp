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
  Driver::ptr driver;
  ImageHandler::ptr handler;
  NetworkDispatcher::ptr dispatcher;
  QTcpServer server;
  DriverForwarder::ptr forwarder;
  void new_connection();
};

NetworkServer::NetworkServer(const Driver::ptr &driver, const ImageHandler::ptr &handler, QObject* parent) : QObject{parent}, dptr(driver, handler, make_shared<NetworkDispatcher>())
{
  d->server.setMaxPendingConnections(1);
  connect(&d->server, &QTcpServer::newConnection, bind(&Private::new_connection, d.get()));
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
  dispatcher->setSocket(server.nextPendingConnection());
  forwarder = make_shared<DriverForwarder>(dispatcher, driver, handler);
}




#include "networkserver.moc"
