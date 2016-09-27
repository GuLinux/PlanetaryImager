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
#ifndef UTILS_H
#define UTILS_H

#include <QVector>
#include <QElapsedTimer>
#include <functional>
#include <QDebug>
#include <QString>

#define dbg_print_thread_id \
  static bool printed = false; \
  if(!printed) { \
    qDebug() << __PRETTY_FUNCTION__ << ": thread_id=" << QThread::currentThreadId(); \
    printed = true; \
  }

static constexpr double BITS_8_TO_16 = 256.;
static constexpr double BITS_16_TO_8 = 1./256.;
class LogScope {
public:
  LogScope(const QString &fname, const QString &enter = "ENTER", const QString &exit = "EXIT") : fname{fname}, exit{exit} {
    qDebug() << enter << " " << fname;
  }
  ~LogScope() {
    qDebug() << exit << " " << fname;
  }
private:
  const QString fname;
  const QString exit;
};
  template<typename T> class LogClassScope : public LogScope {
  public:
    LogClassScope() : LogScope(typeid(T).name(), "Create Object", "Delete Object") {}
  };

#define LOG_F_SCOPE LogScope log_current_scope(__PRETTY_FUNCTION__);
#define LOG_C_SCOPE(Class) LogClassScope<Class> log_current_class
  
#endif
