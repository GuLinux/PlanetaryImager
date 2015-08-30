#include "driver.h"
#include "qhy/qhydriver.h"
#include "simulator/simulatordriver.h"

using namespace std;

typedef QList<shared_ptr<Driver>> Drivers;

class SupportedDrivers::Private {
public:
  Private(const Drivers &drivers, SupportedDrivers *q);
  Drivers drivers;
  
private:
  SupportedDrivers *q;
};

SupportedDrivers::Private::Private(const Drivers& drivers, SupportedDrivers* q) : drivers{drivers}, q{q}
{

}

SupportedDrivers::SupportedDrivers() : dptr({
  make_shared<QHYDriver>(),
#ifdef HAVE_SIMULATOR
  make_shared<SimulatorDriver>(),
#endif
//  make_shared<QMultimediaDriver>(),
  
  }, this)
{

}

SupportedDrivers::~SupportedDrivers()
{
}


Driver::Cameras SupportedDrivers::cameras() const
{
  Cameras cameras;
  qDebug() << "drivers: " << d->drivers.size();
  for(auto driver: d->drivers) {
    qDebug() << "driver cameras: " << driver->cameras().size();
    cameras.append(driver->cameras());
  }
  return cameras;
}
