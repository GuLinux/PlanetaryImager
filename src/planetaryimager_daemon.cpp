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
#include "planetaryimager.h"
#include "Qt/qt_strings_helper.h"
#include "commons/commandline.h"
#include "commons/frame.h"
#include "network/networkdispatcher.h"

using namespace std;


int main(int argc, char** argv)
{
    qRegisterMetaType<FramePtr>("FramePtr");
    qRegisterMetaType<FrameConstPtr>("FrameConstPtr");
    CrashHandler crash_handler({SIGSEGV, SIGABRT});
    cerr << "Starting PlanetaryImager Daemon - version " << PLANETARY_IMAGER_VERSION << " (" << HOST_PROCESSOR << ")" << endl;
    QCoreApplication app(argc, argv);
    app.setApplicationName("PlanetaryImager-Daemon");
    app.setApplicationVersion(PLANETARY_IMAGER_VERSION);

    CommandLine commandLine(app);
    commandLine.daemon("0.0.0.0").process();

    LogHandler log_handler{commandLine};

    Configuration configuration;
    auto driver = make_shared<SupportedDrivers>(commandLine.driversDirectories());
    auto dispatcher = make_shared<NetworkDispatcher>();
    auto save_images = make_shared<LocalSaveImages>(configuration);
    auto frames_forwarder = make_shared<FramesForwarder>(dispatcher);
    auto imageHandlers = make_shared<ImageHandlers>(QList<ImageHandlerPtr>{frames_forwarder, save_images});
    auto configuration_forwarder = make_shared<ConfigurationForwarder>(configuration, dispatcher);
    auto save_files_forwarder = make_shared<SaveFileForwarder>(save_images, dispatcher);
    auto planetaryImager = make_shared<PlanetaryImager>(driver, imageHandlers, save_images, configuration);
    auto server = make_shared<NetworkServer>(planetaryImager, dispatcher, frames_forwarder);
    QObject::connect(save_files_forwarder.get(), &SaveFileForwarder::isRecording, frames_forwarder.get(), &FramesForwarder::recordingMode);

    QObject::connect(planetaryImager.get(), &PlanetaryImager::cameraConnected, save_files_forwarder.get(), [&]{
      save_files_forwarder->setImager(planetaryImager->imager());
    });
    QObject::connect(planetaryImager.get(), &PlanetaryImager::cameraDisconnected, save_files_forwarder.get(), [&]{
      save_files_forwarder->setImager(nullptr);
    });


    QMetaObject::invokeMethod(server.get(), "listen", Q_ARG(QString, commandLine.address()), Q_ARG(int, commandLine.port()));

    return app.exec();
}
