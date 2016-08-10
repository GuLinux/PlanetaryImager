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
