#ifndef QHYDRIVER_H
#define QHYDRIVER_H
#include "dptr.h"

#include "drivers/driver.h"

class QHYDriver : public Driver
{
public:
    QHYDriver();
    ~QHYDriver();
    virtual Cameras cameras() const;
    static QString error_name(int code);
    virtual ImagerPtr imager(Camera camera, const QList< ImageHandlerPtr >& imageHandlers);
    
  class error : public std::runtime_error {
  public:
    error(const QString &label, int code);
  };
private:
  D_PTR
};

#endif // QHYDRIVER_H
