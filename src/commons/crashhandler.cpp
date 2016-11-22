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

#include "crashhandler.h"
#include "c++/backtrace.h"
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <QtGlobal>

using namespace std;

namespace {
#ifndef Q_OS_WIN32
void crash_handler(int sig) {
  fprintf(stderr, "Error: signal %d:\n", sig);
  cerr << GuLinux::Backtrace::backtrace(50, 1);
  exit(1);
}
#endif
}

CrashHandler::CrashHandler(const std::initializer_list<int>& install_signals) : CrashHandler{list<int>(install_signals)}
{
}

CrashHandler::CrashHandler(const std::list<int> &install_signals)
{
#ifndef Q_OS_WIN32
  for(auto sig: install_signals)
    signal(sig, crash_handler);
#endif
}

CrashHandler::~CrashHandler()
{
}
