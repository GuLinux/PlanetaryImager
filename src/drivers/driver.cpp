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
#include "driver.h"
#include "qhy/qhydriver.h"
#include "simulator/simulatordriver.h"
#include "available_drivers.h"

using namespace std;

class SupportedDrivers::Private {
public:
  Private(SupportedDrivers *q);  
private:
  SupportedDrivers *q;
};

SupportedDrivers::Private::Private(SupportedDrivers* q) : q{q}
{

}

SupportedDrivers::SupportedDrivers() : dptr(this)
{
  static bool metatypes_registered = false;
  if(!metatypes_registered) {
    metatypes_registered = true;
    qRegisterMetaType<ImagerPtr>("ImagerPtr");
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
  return cameras;
}
