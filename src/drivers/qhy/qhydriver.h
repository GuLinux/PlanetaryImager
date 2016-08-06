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
    
  class error : public std::runtime_error {
  public:
    error(const QString &label, int code);
  };
private:
  DPTR
};

#endif // QHYDRIVER_H
