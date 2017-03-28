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
  void find_drivers(const QString &directory);
  void load_driver(const QString &filename);
};


SupportedDrivers::SupportedDrivers(const QStringList &driversPath) : dptr(this)
{
  for(const QString &path: driversPath)
    d->find_drivers(path);
}

void SupportedDrivers::Private::find_drivers(const QString& directory)
{
  qDebug() << "Looking for drivers in " << directory;
  if(QFileInfo{directory}.isDir()) {
    QDirIterator it{directory, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks};
    while(it.hasNext()) {
      load_driver(it.next());
    }
  }
}


void SupportedDrivers::Private::load_driver(const QString& filename)
{
  auto plugin = make_shared<QPluginLoader>(filename);
  auto getClassName = [](const auto &p) { return p->metaData().value("className"); };
  if(plugin->metaData().value("IID").toString() == DRIVER_IID && 
        find_if(drivers.begin(), drivers.end(), [&](const auto &p) { return getClassName(p) == getClassName(plugin); }) == drivers.end() ) {
    if(plugin->load()) {
      qDebug() << "driver " << plugin->fileName() << "loaded:" << QJsonDocument{plugin->metaData()}.toJson();
      drivers.push_back(plugin);
    } else {
      qWarning() << "Error loading driver " << plugin->fileName() << ": " << plugin->errorString();
    }
  }
}


SupportedDrivers::~SupportedDrivers()
{
}


Driver::Cameras SupportedDrivers::cameras() const
{
  Cameras cameras;
  list<Driver*> drivers;
  transform(begin(d->drivers), end(d->drivers), back_inserter(drivers), [](const auto &p) { return qobject_cast<Driver*>(p->instance()); });
  for(auto driver: drivers) {
    if(driver)
      cameras.append(driver->cameras());
  }

  return cameras;
}
