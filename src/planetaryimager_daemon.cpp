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
#include "planetaryimager_mainwindow.h"
#include "commons/version.h"
#include <iostream>
#include <QDebug>
#include <QCommandLineParser>
#include "commons/crashhandler.h"
#include "commons/loghandler.h"
#include "network/server/networkserver.h"
#include "network/server/configurationforwarder.h"
#include "image_handlers/backend/local_saveimages.h"
#include "network/server/savefileforwarder.h"
#include "network/server/framesforwarder.h"
#include "drivers/supporteddrivers.h"

#include "Qt/strings.h"
using namespace std;


int main(int argc, char** argv)
{
    qRegisterMetaType<Frame::ptr>("Frame::ptr");
    CrashHandler crash_handler({SIGSEGV, SIGABRT});
    cerr << "Starting PlanetaryImager Daemon - version " << PLANETARY_IMAGER_VERSION << " (" << HOST_PROCESSOR << ")" << endl;
    QCoreApplication app(argc, argv);
    LogHandler log_handler;
    app.setApplicationName("PlanetaryImager");
    app.setApplicationVersion(PLANETARY_IMAGER_VERSION);
    auto configuration = make_shared<Configuration>();
    auto driver = make_shared<SupportedDrivers>();
    auto dispatcher = make_shared<NetworkDispatcher>();
    auto save_images = make_shared<LocalSaveImages>(configuration);
    auto frames_forwarder = make_shared<FramesForwarder>(dispatcher);
    auto imageHandlers = ImageHandler::ptr{new ImageHandlers{
      frames_forwarder,
      save_images,
    }};
    auto configuration_forwarder = make_shared<ConfigurationForwarder>(configuration, dispatcher);
    auto save_files_forwarder = make_shared<SaveFileForwarder>(save_images, dispatcher);
    auto server = make_shared<NetworkServer>(driver, imageHandlers, dispatcher, save_files_forwarder, frames_forwarder );
    
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
      { {"p", "port"}, "listening port for server (default: %1)"_q % Configuration::DefaultServerPort, "port", "%1"_q % Configuration::DefaultServerPort},
    });
    parser.process(app);
    
    bool port_ok;
    int port = parser.value("port").toInt(&port_ok);
    if(!port_ok) {
      cerr << "Error: invalid port specified" << endl;
      parser.showHelp(1);
    }
    QMetaObject::invokeMethod(server.get(), "listen", Q_ARG(QString, "0.0.0.0"), Q_ARG(int, port));
    
    return app.exec();
}
