#include "driver.h"
#include "qhy/qhydriver.h"
#include <QRegularExpression>

using namespace std;

typedef QList<shared_ptr<Driver>> Drivers;

class SupportedDrivers::Private {
public:
  Private(const Drivers &drivers, SupportedDrivers *q);
  Drivers drivers;
  QList<QPair<Driver::Camera, DriverPtr>> cameras;
  
private:
  SupportedDrivers *q;
};

SupportedDrivers::Private::Private(const Drivers& drivers, SupportedDrivers* q) : drivers{drivers}, q{q}
{

}

SupportedDrivers::SupportedDrivers() : dpointer({make_shared<QHYDriver>(), }, this)
{

}

SupportedDrivers::~SupportedDrivers()
{
}



ImagerPtr SupportedDrivers::imager(Driver::Camera camera, const QList< ImageHandlerPtr >& imageHandlers)
{
  for(auto c: d->cameras) {
    if(c.first == camera)
      return c.second->imager(camera, imageHandlers);
  }
  return {};
}

Driver::Cameras SupportedDrivers::cameras() const
{
  d->cameras.clear();
  Cameras cameras;
  for(auto driver: d->drivers) {
    for(auto camera: driver->cameras()) {
      cameras.push_back(camera);
      d->cameras.push_back({camera, driver});
    }
  }
  return cameras;
}

QString Driver::Camera::name() const
{
  return QString(id).remove(QRegularExpression{"-[\\da-f]+$"});
}

bool Driver::Camera::operator==(const Driver::Camera& o) const
{
  return QString{o.id} == QString{id} && o.index == index;
}
