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
#include "Qt/qt_strings_helper.h"
#include <QCommandLineParser>
#include <iostream>
#include "loghandler.h"
#include <QStandardPaths>

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(CommandLine) {
  QCoreApplication &app;
  QCommandLineParser parser;
  void loggingOptions();
};
#include <iostream>
void CommandLine::Private::loggingOptions()
{
  auto levels_string = LogHandler::log_levels().values();
  QStringList levels;
  transform(begin(levels_string), end(levels_string), back_inserter(levels), bind(&QString::fromStdString, _1));
  parser.addOption({"console-log-level", "Console logging level (one of %1)"_q % levels.join(", "), "level", QString::fromStdString(LogHandler::log_levels()[QtWarningMsg]) });
  parser.addOption({"log-file", "Log file path", "log_file", "%1/%2.log"_q % QStandardPaths::writableLocation(QStandardPaths::CacheLocation) % app.applicationName() });
}


CommandLine::CommandLine(QCoreApplication& app) : dptr(app)
{
  d->parser.addHelpOption();
  d->parser.addVersionOption();
}

CommandLine::~CommandLine()
{
}


CommandLine & CommandLine::backend()
{

#ifdef OSX_BUNDLE
  QString driversDirectory = QCoreApplication::applicationDirPath() + "/" + DRIVERS_DIRECTORY;
#else
  QString driversDirectory = DRIVERS_DIRECTORY;
#endif
  d->parser.addOption({"drivers", "Drivers directory", "drivers_directory_path", driversDirectory});
  d->loggingOptions();
  return *this;
}

CommandLine & CommandLine::daemon(const QString &listenAddress)
{
  backend();
  d->parser.addOptions({
    { {"p", "port"}, "listening port for server (default: %1)"_q % Configuration::DefaultServerPort, "port", "%1"_q % Configuration::DefaultServerPort},
  });
  d->parser.addOptions({
    { {"a", "address"}, "listening address for server (default: %1)"_q % listenAddress, "address", "%1"_q % listenAddress},
  });
  return *this;
}

CommandLine & CommandLine::scripting()
{
  d->loggingOptions();
  d->parser.addOptions({
    { {"p", "port"}, "server port (default: %1)"_q % Configuration::DefaultServerPort, "port", "%1"_q % Configuration::DefaultServerPort},
  });
  d->parser.addOptions({
    { {"a", "address"}, "server address (default: %1)"_q % "localhost", "address", "%1"_q % "localhost"},
  });
  return *this;
}


CommandLine & CommandLine::frontend()
{
  d->loggingOptions();
  return *this;
}


CommandLine & CommandLine::process()
{
  d->parser.process(d->app);
  return *this;
}



QStringList CommandLine::driversDirectories() const
{
  QStringList drivers(d->parser.value("drivers"));
#ifdef ADDITIONAL_DRIVERS_DIRECTORY
  drivers << ADDITIONAL_DRIVERS_DIRECTORY;
#endif
  return drivers;
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

QtMsgType CommandLine::consoleLogLevel() const
{
  return LogHandler::log_levels().key(d->parser.value("console-log-level").toUpper().toStdString());
}

QString CommandLine::logfile() const
{
  return d->parser.value("log-file");
}

QString CommandLine::address() const
{
  return d->parser.value("address");
}

