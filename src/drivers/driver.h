#ifndef PLANETARY_IMAGER_DRIVER_H
#define PLANETARY_IMAGER_DRIVER_H
#include <QList>
#include <QString>
#include "imager.h"
#include "dptr.h"

class Driver {
public:
  class Camera {
  public:
    virtual QString name() const = 0;
    virtual ImagerPtr imager(const ImageHandlerPtr &imageHandler) const = 0;
  };
  typedef std::shared_ptr<Camera> CameraPtr;
  typedef QList<CameraPtr> Cameras; 
  
  virtual Cameras cameras() const = 0;
};
typedef std::shared_ptr<Driver> DriverPtr;

class SupportedDrivers : public Driver {
public:
  SupportedDrivers();
  ~SupportedDrivers();
  virtual Cameras cameras() const;
private:
  D_PTR;
};

#endif