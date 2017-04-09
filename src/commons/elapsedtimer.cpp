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

#include "elapsedtimer.h"

using namespace std;


DPTR_IMPL(ElapsedTimer) {
  chrono::steady_clock::time_point started;
  chrono::microseconds elapsed;
  bool running = false;
};

ElapsedTimer::ElapsedTimer() : dptr()
{
  reset();
}

ElapsedTimer::~ElapsedTimer()
{
}

chrono::microseconds ElapsedTimer::elapsed() const
{
  if(! d->running)
    return d->elapsed;
  chrono::nanoseconds elapsed = chrono::steady_clock::now() - d->started;
  return d->elapsed + chrono::duration_cast<chrono::milliseconds>(elapsed);
}

long ElapsedTimer::microseconds() const
{
  return elapsed().count();
}

long ElapsedTimer::milliseconds() const
{
  return microseconds() / 1000;
}

long ElapsedTimer::seconds() const
{
  return milliseconds() / 1000;
}


void ElapsedTimer::start()
{
  reset();
  d->running = true;
  d->started = chrono::steady_clock::now();
}

void ElapsedTimer::reset()
{
  d->running = false;
  d->started = {};
  d->elapsed = chrono::microseconds{0};
}

void ElapsedTimer::pause()
{
  d->elapsed += elapsed();
  d->running = false;
}

void ElapsedTimer::resume()
{
  d->running = true;
  d->started = chrono::steady_clock::now();
}
