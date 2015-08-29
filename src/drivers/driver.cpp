#include "driver.h"
#include "qhy/qhydriver.h"
#include "qmultimedia/qmultimediadriver.h"

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
  
  for(auto driver: d->drivers) {
    cameras.append(driver->cameras());
  }
  return cameras;
}
