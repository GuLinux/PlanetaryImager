/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
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

#ifndef EXPOSURETIMER_H
#define EXPOSURETIMER_H

#include <QObject>
#include "c++/dptr.h"

class Imager;
class ExposureTimer : public QObject
{
    Q_OBJECT
public:
  ExposureTimer(QObject *parent = nullptr);
  ~ExposureTimer();
  void set_imager(Imager *imager);
signals:
  void progress(double total, double elapsed, double remaining);
  void finished();
private:
  DPTR
};

#endif // EXPOSURETIMER_H
