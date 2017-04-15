/*
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
#include <QCoreApplication>
#include "commons/version.h"
#include <iostream>
#include <QDebug>
#include "commons/loghandler.h"
//#include "commons/crashhandler.h"
#include "commons/commandline.h"

#include "network/client/networkclient.h"
#include "network/client/scriptingclient.h"
#include "network/networkdispatcher.h"


using namespace std;


int main(int argc, char** argv)
{
//   CrashHandler crash_handler({SIGSEGV, SIGABRT});
  cerr << "PlanetaryImager Scripting - version " << PLANETARY_IMAGER_VERSION << " (" << HOST_PROCESSOR << ")" << endl;
  QCoreApplication app(argc, argv);
  app.setApplicationName("PlanetaryImager-Scripting");
  app.setApplicationVersion(PLANETARY_IMAGER_VERSION);
  
  CommandLine commandLine(app);
  commandLine.scripting().process();
  auto dispatcher = make_shared<NetworkDispatcher>();
  auto client = make_shared<NetworkClient>(dispatcher);
  auto scriptingClient = make_shared<ScriptingClient>(dispatcher);
  QObject::connect(client.get(), &NetworkClient::connected, scriptingClient.get(), &ScriptingClient::console);
  client->connectToHost(commandLine.address(), commandLine.port(), {Configuration::Network_NoImage});
  
  LogHandler log_handler{commandLine};
  
  return app.exec();
}
