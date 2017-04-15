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

#ifndef SCRIPTINGENGINE_H
#define SCRIPTINGENGINE_H

#include <QObject>
#include "c++/dptr.h"
#include "commons/configuration.h"
#include "image_handlers/saveimages.h"
#include "drivers/imager.h"
#include "network/networkdispatcher.h"

class ScriptingEngine : public QObject, public NetworkReceiver
{
    Q_OBJECT
public:
  ScriptingEngine(const Configuration::ptr &configuration, const SaveImages::ptr &saveImages, const NetworkDispatcher::ptr &dispatcher, QObject *parent = nullptr);
  ~ScriptingEngine();
  typedef std::shared_ptr<ScriptingEngine> ptr;
  void setImager(Imager *imager);
public slots:
  void run(const QString &script);
signals:
  void reply(const QString &);
private:
  DPTR
};

#endif // SCRIPTINGENGINE_H
