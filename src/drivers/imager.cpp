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
#include "Qt/qt_strings_helper.h"
#include "commons/utils.h"
#include <QCoreApplication>
#include <QMutex>
using namespace std;
using namespace std::placeholders;

DPTR_IMPL(Imager) {
  const ImageHandler::ptr image_handler;
  ImagerThreadPtr imager_thread;
  LOG_C_SCOPE(Imager);
  unique_ptr<QHash<Imager::Capability, bool>> capabilities;
  bool destroyed = false;
  Configuration::CaptureEndianess captureEndianess = Configuration::CaptureEndianess::CameraDefault;
};

Imager::Imager(const ImageHandler::ptr& image_handler) : QObject(nullptr), dptr(image_handler)
{
  static bool metatypes_registered = false;
  if(!metatypes_registered) {
    metatypes_registered = true;
    qRegisterMetaType<Imager::Control>("Imager::Control");
  }
  connect(this, &Imager::changed, this, [=](const Imager::Control &c) { if(c.is_exposure) update_exposure(); });
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
      if(! changed.same_value(control)) {
        qDebug() << "control " << control << " has changed to " << changed;
        changed_controls.push_back(changed);
      }
    }
  }
  setControls(changed_controls);
}

void Imager::setControls(const Controls& controls)
{
  for(auto control: controls) {
    setControl(control);
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
  if(d->destroyed)
    return;
  d->imager_thread.reset();
  emit disconnected();
  this->deleteLater();
  d->destroyed = true;
}

void Imager::readTemperature()
{
}

void Imager::restart(const ImagerThread::Worker::factory& worker)
{
  LOG_F_SCOPE
  d->imager_thread.reset();
  d->imager_thread = make_shared<ImagerThread>(worker(), this, d->image_handler, d->captureEndianess);
  update_exposure();
  d->imager_thread->start();
}

void Imager::update_exposure()
{
  for(auto control: controls()) {
    if(control.is_duration && control.is_exposure) {
      const chrono::duration<double> exposure = control.seconds();
      qDebug() << "Setting exposure: " << exposure.count();
      if(d->imager_thread)
        d->imager_thread->set_exposure(exposure);
      emit exposure_changed(control);
      break;
    }
  }
}

void Imager::wait_for(const std::shared_ptr<QWaitCondition>& wait_condition) const
{
  if(! wait_condition)
    return;
  QMutex wait_mutex;
  wait_mutex.lock();
  wait_condition->wait(&wait_mutex);
}


shared_ptr<QWaitCondition> Imager::push_job_on_thread(const ImagerThread::Job& job)
{
  if(! d->imager_thread) {
    qWarning() << "Requested job without imager thread started";
    return {};
  }
  return d->imager_thread->push_job(job);
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

void Imager::setCaptureEndianess(Configuration::CaptureEndianess captureEndianess)
{
    d->captureEndianess = captureEndianess;

    if (d->imager_thread)
        wait_for(push_job_on_thread([=]() { d->imager_thread->setCaptureEndianess(d->captureEndianess); }));
}

#include "imager.moc"
