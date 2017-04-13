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
#include "loghandler.h"
#include <iostream>
#include <iomanip>
#include <QDebug>
#include <QHash>
#include <functional>
#include <fstream>
#include <QFileInfo>
#include <QDir>
#include "commons/commandline.h"

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(LogHandler) {
   static Output::list outputs;
   static void log_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
   static Output output(std::ostream &stream, QtMsgType level);
   ofstream logfile;
};

LogHandler::Output::list LogHandler::Private::outputs = {};

QHash<QtMsgType, string> LogHandler::log_levels()
{
  static QHash<QtMsgType, string> levels {
    {QtFatalMsg   , "FATAL"},
    {QtCriticalMsg, "CRITICAL"},
    {QtWarningMsg , "WARNING"},
    {QtDebugMsg   , "DEBUG"},
    {QtInfoMsg   , "INFO"},
  };
  return levels;
}


void LogHandler::Private::log_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  static QHash<QtMsgType, int> priority {
    {QtFatalMsg   , 10},
    {QtCriticalMsg, 20},
    {QtWarningMsg , 30},
    {QtInfoMsg    , 40},
    {QtDebugMsg   , 50},
  };
  for(auto output: outputs) {
#ifndef DEVELOPER_MODE
    if(priority[type] > priority[output.level])
      continue;
#endif
    QString position;
    if(context.file && context.line) {
      position = QString("%1:%2").arg(context.file).arg(context.line).replace(SRC_DIR, "");
    }
    QString function = context.function ? context.function : "";
    output.stream.get() << setw(8) << LogHandler::log_levels()[type] << " - " /*<< qPrintable(position) << "@"*/<< qPrintable(function) << " " << qPrintable(msg) << endl;
    output.stream.get().flush();
  }
}


LogHandler::LogHandler(const CommandLine &commandLine) : dptr()
{
  d->outputs.push_back(LogHandler::Private::output(cerr, commandLine.consoleLogLevel()));
  cerr << "Writing full output to logfile: " << commandLine.logfile().toStdString() << endl;
  QFileInfo{commandLine.logfile()}.dir().mkpath(".");
  d->logfile.open(commandLine.logfile().toStdString(), ios::out | ios::trunc);
  
  d->outputs.push_back(LogHandler::Private::output(d->logfile, QtDebugMsg));
  qInstallMessageHandler(&Private::log_handler);
}

LogHandler::~LogHandler()
{
  qInstallMessageHandler(nullptr);
}


LogHandler::Output LogHandler::Private::output(std::ostream& stream, QtMsgType level)
{
  return {std::ref(stream), level};
}
