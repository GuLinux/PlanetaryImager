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

#ifndef PLANETARYIMAGERSCRIPTINGPROXY_H
#define PLANETARYIMAGERSCRIPTINGPROXY_H

#include <QObject>
#include "c++/dptr.h"
#include "planetaryimager.h"
#include <QtQml/QJSValue>
#include <QtQml/QJSEngine>

class PlanetaryImagerScriptingProxy : public QObject
{
    Q_OBJECT
public:
    PlanetaryImagerScriptingProxy(const PlanetaryImager::ptr &planetaryImager, QJSEngine *engine);
    ~PlanetaryImagerScriptingProxy();
    QJSValue jsValue() const;
public slots:
  QJSValue cameras();
private:
  DPTR
};

#endif // PLANETARYIMAGERSCRIPTINGPROXY_H
