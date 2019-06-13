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

#include "exposuretimer.h"
#include <QTimer>
#include <functional>
#include <QElapsedTimer>
#include "drivers/imager.h"

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(ExposureTimer) {
  QTimer *timer;
  ExposureTimer *q;
  QElapsedTimer elapsed;
  double exposure;
  void started(double exposure);
  void progress();
  void finished();
};

ExposureTimer::ExposureTimer(QObject* parent) : QObject{parent}, dptr(new QTimer{this}, this)
{
  d->timer->setSingleShot(false);
  d->timer->setInterval(100);
  connect(d->timer, &QTimer::timeout, this, bind(&Private::progress, d.get()));
}

ExposureTimer::~ExposureTimer()
{
}

void ExposureTimer::set_imager(Imager* imager)
{
  if(!imager)
    return;
  connect(imager, &Imager::long_exposure_started, this, bind(&Private::started, d.get(), _1), Qt::QueuedConnection);
  connect(imager, &Imager::long_exposure_ended, this, bind(&Private::finished, d.get()), Qt::QueuedConnection);
}


void ExposureTimer::Private::progress()
{
  double elapsed = static_cast<double>(this->elapsed.elapsed()) / 1000.;
  emit q->progress(exposure, elapsed, exposure - elapsed);
}


void ExposureTimer::Private::started(double exposure)
{
  this->exposure = exposure;
  elapsed.restart();
  progress();
  timer->start();
}

void ExposureTimer::Private::finished()
{
  timer->stop();
  emit q->finished();
}
