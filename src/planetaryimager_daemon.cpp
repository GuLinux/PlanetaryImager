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
#include "c++/backtrace.h"
#include <unistd.h>
#include <signal.h>
#include "commons/loghandler.h"
using namespace std;


void crash_handler(int sig) {
  fprintf(stderr, "Error: signal %d:\n", sig);
  cerr << GuLinux::Backtrace::backtrace(50, 1);
  exit(1);
}

int main(int argc, char** argv)
{
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
    cerr << "Starting PlanetaryImager Daemon - version " << PLANETARY_IMAGER_VERSION << " (" << HOST_PROCESSOR << ")" << endl;
    QCoreApplication app(argc, argv);
    LogHandler log_handler;
    app.setApplicationName("PlanetaryImager");
    app.setApplicationVersion(PLANETARY_IMAGER_VERSION);
    // TODO: initialize all handlers and drivers here
    //PlanetaryImagerMainWindow mainWindow;
    //mainWindow.show();
    return app.exec();
}
