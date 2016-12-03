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

#ifndef NETWORKDISPATCHER_H
#define NETWORKDISPATCHER_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include "c++/dptr.h"
#include "networkpacket.h"

class NetworkReceiver {
public:
  virtual void handle(const NetworkPacket::ptr &packet) = 0;
};

class NetworkDispatcher : public QObject
{
  Q_OBJECT
public:
  typedef std::shared_ptr<NetworkDispatcher> ptr;
  NetworkDispatcher(QObject *parent = nullptr);
  ~NetworkDispatcher();
  void attach(NetworkReceiver *receiver);
  void detach(NetworkReceiver *receiver);
  void setSocket(QTcpSocket *socket);
public slots:
  void send(const NetworkPacket::ptr &packet);
private:
  DPTR
};

#endif // NETWORKDISPATCHER_H
