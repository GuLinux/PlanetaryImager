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

#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H
#include <QObject>
#include "c++/dptr.h"
#include "drivers/driver.h"
#include "network/server/savefileforwarder.h"
#include "network/networkdispatcher.h"
#include "framesforwarder.h"
class NetworkServer : public QObject, public NetworkReceiver
{
    Q_OBJECT
public:
  NetworkServer(const Driver::ptr &driver, const ImageHandler::ptr &handler, const NetworkDispatcher::ptr &dispatcher, const SaveFileForwarder::ptr &save_file, const FramesForwarder::ptr &framesForwarder, QObject *parent = nullptr);
  ~NetworkServer();
public slots:
  void listen(const QString &address, int port);
private:
  DPTR
};

#endif // NETWORKSERVER_H
