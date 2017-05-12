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
#include <QDebug>
#include "protocol/scriptingprotocol.h"
#include <QtConcurrent/QtConcurrent>
#include <cstdio>
#include <cstdlib>
extern "C" {
#include <readline/readline.h>
#include <readline/history.h>
}
using namespace std;

DPTR_IMPL(ScriptingClient) {
  NetworkDispatcher::ptr dispatcher;
  unique_ptr<QTextStream> outStream;
  ScriptingClient *q;
};

ScriptingClient::~ScriptingClient()
{
}

ScriptingClient::ScriptingClient(const NetworkDispatcher::ptr &dispatcher, QObject *parent)
: QObject{parent}, NetworkReceiver(dispatcher), dptr(dispatcher, make_unique<QTextStream>(stdout, QIODevice::WriteOnly), this)
{
  register_handler(ScriptingProtocol::ScriptReply, [this](const NetworkPacket::ptr &packet) {
    *d->outStream << packet->payloadVariant().toString() << "\n";
    d->outStream->flush();
  });
}

void ScriptingClient::console()
{
  qDebug() << "starting scripting console";
  QtConcurrent::run([=]{
    QString line;
    QTextStream input{stdin, QIODevice::ReadOnly};
    QStringList lines;
    
    auto readLine = [&] {
      char *cline = readline("> ");
      if(cline == nullptr)
        return false;
      line = QString{cline};
      if(! line.isEmpty())
        add_history(cline);
      free(cline);
      return true;
    };
    
    while( readLine() && line != "quit" && line != "exit") {
      if(line.endsWith("\\")) {
        lines << line.left(line.length()-2);
        continue;
      }
      if(! line.isEmpty()) {
        lines << line;
        QMetaObject::invokeMethod(this, "sendScript", Qt::QueuedConnection, Q_ARG(QString, lines.join("\n")));
        lines.clear();
      }
    }
    qApp->quit();
  });
}

void ScriptingClient::sendScript(const QString& script)
{
  d->dispatcher->send(ScriptingProtocol::packetScript() << script);
}
