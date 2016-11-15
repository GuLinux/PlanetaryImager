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
#include <QMutex>
using namespace std;
using namespace std::placeholders;

DPTR_IMPL(Imager) {
  const ImageHandler::ptr image_handler;
  ImagerThread::ptr imager_thread;
  LOG_C_SCOPE(Imager);
  unique_ptr<QHash<Imager::Capability, bool>> capabilities;
};

Imager::Imager(const ImageHandler::ptr& image_handler) : QObject(nullptr), dptr(image_handler)
{
}

Imager::~Imager()
{
  destroy();
}

void Imager::import_controls(const QVariantList& controls, bool by_id)
{
  qDebug() << "Importing controls: " << controls << ", by id: " << by_id;
  map<QVariant, QVariantMap> controls_mapped;
  transform(begin(controls), end(controls), inserter(controls_mapped, controls_mapped.begin()), [&](const QVariant &item){
    QVariantMap map = item.toMap();
    return make_pair(by_id ? map["id"] : map["name"], map);
  });
  auto device_controls = this->controls();
  Controls changed_controls;
  for(auto &control: device_controls) {
    QVariant key = by_id ? QVariant{static_cast<qlonglong>(control.id)} : QVariant{control.name};
    if(controls_mapped.count(key)) {
      Control changed = control;
      qDebug() << "Importing control: " << control.id << ", " << control.name;
      changed.import(controls_mapped[key]);
      if(! changed.same_value(control))
        changed_controls.push_back(changed);
    }
  }
  setControls(changed_controls);
}

void Imager::setControls(const Controls& controls)
{
  for(auto control: controls) {
    if(auto wait_condition = setControl(control)) {
      qDebug() << "Waiting for control: " << control;
      QMutex wait_condition_mutex;
      wait_condition->wait(&wait_condition_mutex);
    }
  }
}


QVariantList Imager::export_controls() const
{
  QVariantList controls_qvariant;
  auto controls = this->controls();
  transform(controls.begin(), controls.end(), back_inserter(controls_qvariant), bind(&Control::asMap, _1));
  return controls_qvariant;
}




void Imager::destroy()
{
  d->imager_thread.reset();
  emit disconnected();
  this->deleteLater();
}

void Imager::restart(const ImagerThread::Worker::factory& worker)
{
  LOG_F_SCOPE
  d->imager_thread.reset();
  d->imager_thread = make_shared<ImagerThread>(worker(), this, d->image_handler);
  d->imager_thread->start();
}

void Imager::set_exposure(const Control &control)
{
  if(! control.is_duration)
    return;
  const chrono::duration<double> exposure = control.seconds();
  if(d->imager_thread)
    d->imager_thread->set_exposure(exposure);
}



void Imager::push_job_on_thread(const ImagerThread::Job& job)
{
  if(! d->imager_thread) {
    qWarning() << "Requested job without imager thread started";
    return;
  }
  d->imager_thread->push_job(job);
}

bool Imager::supports(Capability capability) const
{
  if(! d->capabilities) {
    d->capabilities = make_unique<QHash<Capability, bool>>();
    for(auto capability: properties().capabilities)
      (*d->capabilities)[capability] = true;
  }
  return d->capabilities->value(capability, false);
}

