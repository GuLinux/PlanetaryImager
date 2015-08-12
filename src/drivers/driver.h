#ifndef PLANETARY_IMAGER_DRIVER_H
#define PLANETARY_IMAGER_DRIVER_H
#include <QList>
#include <QString>
#include "imager.h"
#include "dptr.h"

class Driver {
public:

  // TODO: proper class with factory for camera
  struct Camera {
    int index;
    char id[255];
    QString name() const;
    bool operator ==(const Camera& o) const;
  };
  typedef QList<Camera> Cameras; 
  virtual Cameras cameras() const = 0;
  virtual ImagerPtr imager(Camera camera, const QList<ImageHandlerPtr> &imageHandlers) = 0;
};
typedef std::shared_ptr<Driver> DriverPtr;

class SupportedDrivers : public Driver {
public:
  SupportedDrivers();
  ~SupportedDrivers();
  virtual Cameras cameras() const;
  virtual ImagerPtr imager(Camera camera, const QList< ImageHandlerPtr >& imageHandlers);
private:
  D_PTR;
};

#endif