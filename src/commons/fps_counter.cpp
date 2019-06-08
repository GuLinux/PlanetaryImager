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
#include "commons/fps_counter.h"
#include <QDebug>
#include <QTimer>
#include <QElapsedTimer>
using namespace std;

class fps_counter::Private
{
public:
  Private ( const OnFPS &onfps, Mode mode, int fps_trigger_milliseconds, bool mean, fps_counter *q );
  OnFPS onfps;
  Mode mode;
  int fps_trigger_milliseconds;
  QElapsedTimer count_time;
  QElapsedTimer total_time;
  uint64_t frames = 0;
  void timeout();
  bool mean;

private:
  fps_counter *q;
};

fps_counter::Private::Private ( const OnFPS& onfps, fps_counter::Mode mode, int fps_trigger_milliseconds, bool mean, fps_counter* q )
  : onfps{onfps}, mode{mode}, fps_trigger_milliseconds{fps_trigger_milliseconds}, mean{mean}, q{q}
{
}

fps_counter::~fps_counter()
{
}

void fps_counter::Private::timeout()
{
  double elapsed = static_cast<double>(mean ? total_time.elapsed() : count_time.elapsed());
  double fps = static_cast<double>(frames) * 1000/(elapsed);
  onfps(fps);
  count_time.restart();
  if(!mean) {
    frames = 0;
  }
}

fps_counter::fps_counter ( const OnFPS& onFPS, fps_counter::Mode mode, int fps_trigger_milliseconds, bool mean, QObject* parent ) 
  : QObject ( parent ), dptr ( onFPS, mode, fps_trigger_milliseconds, mean, this )
{
  if(mode == Timer) {
    QTimer *timer = new QTimer{this};
    connect(timer, &QTimer::timeout, bind(&Private::timeout, d.get()));
    timer->start(fps_trigger_milliseconds);
  }
  d->count_time.start();
  if(mean)
    d->total_time.start();
}


fps_counter& fps_counter::operator++()
{
  ++d->frames;
  if(d->mode == Elapsed && d->count_time.elapsed() >= d->fps_trigger_milliseconds)
    d->timeout();
  return *this;
}
