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

#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include "c++/dptr.h"
#include "network/protocol/protocol.h"
#include "network/networkreceiver.h"
#include "commons/fwd.h"

FWD_PTR(NetworkDispatcher)

class NetworkClient : public QObject, public NetworkReceiver
{
    Q_OBJECT
public:
  typedef std::shared_ptr<NetworkClient> ptr;
  NetworkClient(const NetworkDispatcherPtr &dispatcher, QObject *parent = nullptr);
  ~NetworkClient();
  enum Status { Connecting, Connected, Disconnected, Error};
public slots:
  void connectToHost(const QString &host, int port, const NetworkProtocol::FormatParameters &parameters);
  void disconnectFromHost();
signals:
  void connected();
  void statusChanged(Status);
  void error(const QString &message);
private:
  DPTR
};

#endif // NETWORKCLIENT_H
