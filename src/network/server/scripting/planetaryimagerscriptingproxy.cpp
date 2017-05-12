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

#include "planetaryimagerscriptingproxy.h"
#include <QCoreApplication>
#include <QDebug>

DPTR_IMPL(PlanetaryImagerScriptingProxy) {
  PlanetaryImager::ptr planetaryImager;
  QJSEngine *engine;
  PlanetaryImagerScriptingProxy *q;
  QJSValue jsValue;
};

PlanetaryImagerScriptingProxy::PlanetaryImagerScriptingProxy(const PlanetaryImager::ptr &planetaryImager, QJSEngine *engine) : QObject{engine}, dptr(planetaryImager, engine, this)
{
  d->jsValue = engine->newQObject(this);
}

PlanetaryImagerScriptingProxy::~PlanetaryImagerScriptingProxy()
{
}

QJSValue PlanetaryImagerScriptingProxy::jsValue() const
{
  return d->jsValue;
}

QJSValue PlanetaryImagerScriptingProxy::cameras()
{
  bool gotCameras = false;
  auto gotCamerasCheck = [&gotCameras] { gotCameras = true; };
  auto connection = connect(d->planetaryImager.get(), &PlanetaryImager::camerasChanged, this, gotCamerasCheck);
  d->planetaryImager->scanCameras();
  while(!gotCameras)
    qApp->processEvents();
  d->planetaryImager->disconnect(connection);
  QVariantList cameras;
  int index = 0;
  for(auto camera: d->planetaryImager->cameras()) {
    QVariantMap cameraProperties{ {"index", index++}, {"name", camera->name()} };
    cameras.push_back(cameraProperties);
  }
  return d->engine->toScriptValue(cameras);
}
