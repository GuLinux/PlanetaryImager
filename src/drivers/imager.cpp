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

#include "imager.h"
#include "Qt/strings.h"
#include "commons/utils.h"
#include <QCoreApplication>
using namespace std;

DPTR_IMPL(Imager) {
  const ImageHandler::ptr image_handler;
  ImagerThread::ptr imager_thread;
  LOG_C_SCOPE(Imager);
};

Imager::Imager(const ImageHandler::ptr& image_handler) : QObject(nullptr), dptr(image_handler)
{
}

Imager::~Imager()
{
  destroy();
}


void Imager::destroy()
{
  d->imager_thread.reset();
  emit disconnected();
  this->deleteLater();
}

void Imager::restart(const ImagerThread::Worker::factory& worker)
{
  d->imager_thread.reset();
  d->imager_thread = make_shared<ImagerThread>(worker(), this, d->image_handler);
  d->imager_thread->start();
}


void Imager::push_job_on_thread(const ImagerThread::Job& job)
{
  if(! d->imager_thread) {
    qWarning() << "Requested job without imager thread started";
    return;
  }
  d->imager_thread->push_job(job);
}
