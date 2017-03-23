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

#include "commandline.h"
#include <QCoreApplication>
#include "commons/configuration.h"
#include "Qt/strings.h"
#include <QCommandLineParser>
#include <iostream>

using namespace std;

DPTR_IMPL(CommandLine) {
  QCoreApplication &app;
  CommandLine *q;
  QCommandLineParser parser;
};

CommandLine::CommandLine(QCoreApplication& app) : dptr(app, this)
{
  d->parser.addHelpOption();
  d->parser.addVersionOption();
}

CommandLine::~CommandLine()
{
}


CommandLine & CommandLine::backend()
{
  d->parser.addOption({"drivers", "Drivers directory", "drivers_directory_path"});
  return *this;
}

CommandLine & CommandLine::daemon()
{
  backend();
  d->parser.addOptions({
    { {"p", "port"}, "listening port for server (default: %1)"_q % Configuration::DefaultServerPort, "port", "%1"_q % Configuration::DefaultServerPort},
  });
  return *this;
}


CommandLine & CommandLine::process()
{
  d->parser.process(d->app);
}



QString CommandLine::driversDirectory() const
{
  return d->parser.value("drivers");
}

int CommandLine::port() const
{
  bool port_ok;
  int port = d->parser.value("port").toInt(&port_ok);
  if(!port_ok) {
    cerr << "Error: invalid port specified" << endl;
    d->parser.showHelp(1);
  }
  return port;
}
