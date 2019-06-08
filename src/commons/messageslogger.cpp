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

#include "messageslogger.h"

DPTR_IMPL(MessagesLogger) {
};

MessagesLogger::MessagesLogger(QObject* parent) : QObject{parent}, dptr()
{
    static bool metatypes_registered = false;
    if(!metatypes_registered) {
      metatypes_registered = true;
      qRegisterMetaType<MessagesLogger::Type>("MessagesLogger::Type");
    }
}

MessagesLogger::~MessagesLogger()
{
}

void MessagesLogger::queue(MessagesLogger::Type type, const QString& title, const QString& message)
{
  QDateTime timestamp = QDateTime::currentDateTime();
  MessagesLogger::instance()->message(timestamp, type, title, message);
}

MessagesLogger * MessagesLogger::instance()
{
  static MessagesLogger *instance = new MessagesLogger;
  return instance;
}
