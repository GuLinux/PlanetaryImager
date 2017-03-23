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

#include "supporteddrivers.h"
#include "available_drivers.h"
#include <QPluginLoader>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QJsonDocument>

using namespace std;

DPTR_IMPL(SupportedDrivers) {
  SupportedDrivers *q;
  QList<shared_ptr<QPluginLoader>> drivers;
};


SupportedDrivers::SupportedDrivers(const QString &driversPath) : dptr(this)
{
  qDebug() << "Looking for drivers in " << driversPath;
  if(QFileInfo{driversPath}.isDir()) {
    QDirIterator it{driversPath, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks};
    while(it.hasNext()) {
      auto plugin = make_shared<QPluginLoader>(it.next());
      if(plugin->metaData().value("IID").toString() == DRIVER_IID && plugin->load()) {
        d->drivers.push_back(plugin);
      }
    }
  }
}

SupportedDrivers::~SupportedDrivers()
{
}


Driver::Cameras SupportedDrivers::cameras() const
{
  Cameras cameras;
  qDebug() << "drivers: " << AvailableDrivers::drivers.size();
  for(auto driver: AvailableDrivers::drivers) {
    qDebug() << "driver cameras: " << driver->cameras().size();
    cameras.append(driver->cameras());
  }
  list<Driver*> drivers;
  transform(begin(d->drivers), end(d->drivers), back_inserter(drivers), [](const auto &p) { return qobject_cast<Driver*>(p->instance()); });
  for(auto driver: drivers) {
    if(driver)
      cameras.append(driver->cameras());
  }

  return cameras;
}
