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

#include "simulatordriver.h"
#include <QDebug>
#include <QMutexLocker>
#include "simulatorimager.h"
#include "serimager.h"

using namespace std;

class SimulatorCamera : public Driver::Camera {
public:
  typedef function<Imager *(const ImageHandler::ptr& imageHandler)> ImageHandlerFactory;
  typedef shared_ptr<SimulatorCamera> ptr;
  SimulatorCamera(const QString &name, const ImageHandlerFactory &factory);
  virtual Imager * imager ( const ImageHandler::ptr& imageHandler) const;
  virtual QString name() const { return m_name; }
private:
  const QString m_name;
  const ImageHandlerFactory factory;
};

SimulatorCamera::SimulatorCamera(const QString& name, const ImageHandlerFactory& factory) : m_name{name}, factory{factory}
{
}




Imager * SimulatorCamera::imager ( const ImageHandler::ptr& imageHandler ) const
{
  return factory(imageHandler);
}

Driver::Cameras SimulatorDriver::cameras() const
{
  static Cameras _cameras {
    make_shared<SimulatorCamera>("Simulator: Planet", [](const ImageHandler::ptr& imageHandler){ return new SimulatorImager(imageHandler); }),
    make_shared<SimulatorCamera>("Simulator: SER file", [](const ImageHandler::ptr& imageHandler){ return new SERImager(imageHandler); }),
  };
  return _cameras;
}


SimulatorDriver::SimulatorDriver()
{
  Q_INIT_RESOURCE(simulator);
}



#include "simulatordriver.moc"
