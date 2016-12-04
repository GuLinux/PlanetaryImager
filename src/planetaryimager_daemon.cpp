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
#include "image_handlers/all_handlers.h"
#include "network/server/framesforwarder.h"
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
    auto driver = make_shared<SupportedDrivers>();
    auto server = make_shared<NetworkServer>(driver, ImageHandler::ptr{new ImageHandlers{}} ); // TODO: add handlers
    QMetaObject::invokeMethod(server.get(), "listen", Q_ARG(QString, "0.0.0.0"), Q_ARG(int, 9999));
    
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    
    // TODO: initialize all handlers and drivers here
    //PlanetaryImagerMainWindow mainWindow;
    //mainWindow.show();
    return app.exec();
}
