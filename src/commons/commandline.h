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

#ifndef COMMANDLINE_H
#define COMMANDLINE_H
#include "c++/dptr.h"
#include <QString>
class QCoreApplication;

class CommandLine
{
public:
  explicit CommandLine(QCoreApplication &app);
  ~CommandLine();
  CommandLine &backend();
  CommandLine &daemon(const QString &listenAddress = "0.0.0.0");
  CommandLine &frontend();
  CommandLine &scripting();
  
  CommandLine &process();
  
  QStringList driversDirectories() const;
  int port() const;
  QString logfile() const;
  QtMsgType consoleLogLevel() const;
  QString address() const;
private:
  DPTR
};

#endif // COMMANDLINE_H
