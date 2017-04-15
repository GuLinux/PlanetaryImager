/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
 * Copyright (C) 2017  Marco Gulino <marco@gulinux.net>
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

#ifndef SCRIPTINGCLIENT_H
#define SCRIPTINGCLIENT_H

#include "network/networkdispatcher.h"
#include "c++/dptr.h"

class ScriptingClient : public QObject, public NetworkReceiver
{
  Q_OBJECT
public:
    ScriptingClient(const NetworkDispatcher::ptr &dispatcher, QObject *parent = nullptr);
    ~ScriptingClient();
    typedef std::shared_ptr<ScriptingClient> ptr;
public slots:
  void console();
  void sendScript(const QString &script);
private:
  DPTR
};

#endif // SCRIPTINGCLIENT_H
