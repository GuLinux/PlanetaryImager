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

#include "scriptingclient.h"
#include <iostream>
#include "protocol/scriptingprotocol.h"
using namespace std;

DPTR_IMPL(ScriptingClient) {
  NetworkDispatcher::ptr dispatcher;
  unique_ptr<QTextStream> stream;
  ScriptingClient *q;
};

ScriptingClient::~ScriptingClient()
{
}

ScriptingClient::ScriptingClient(const NetworkDispatcher::ptr &dispatcher, QObject *parent)
  : QObject{parent}, NetworkReceiver(dispatcher), dptr(dispatcher, make_unique<QTextStream>(stdout), this)
{
  register_handler(ScriptingProtocol::ScriptReply, [this](const NetworkPacket::ptr &packet) {
    *d->stream << packet->payloadVariant().toString() << "\n";
  });
}

void ScriptingClient::console()
{
  QString line;
  while(line != "quit" && line != "exit") {
    if(! line.isEmpty())
      d->dispatcher->send(ScriptingProtocol::packetScript() << line);
    line = d->stream->readLine();
  }
}
