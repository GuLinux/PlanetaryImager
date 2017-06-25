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
#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

#include <QObject>
#include "dptr.h"
#include <functional>

class fps_counter : public QObject
{
  Q_OBJECT
public:
  typedef std::function<void(double fps)> OnFPS;
  enum Mode { Timer, Elapsed };
  ~fps_counter();
  fps_counter(const OnFPS& onFPS, Mode mode = Timer, int fps_trigger_milliseconds = 1000, bool mean = false, QObject* parent = 0);
  fps_counter &operator++();

private:
    DPTR
};

#endif // FPS_COUNTER_H
