#ifndef QHYDRIVER_H
#define QHYDRIVER_H
#include "dptr.h"

#include <QList>
#include <QString>
class QHYDriver
{
public:
    QHYDriver();
    ~QHYDriver();
    struct Camera {
      int index;
      char id[255];
      QString name() const;
    };
    typedef QList<Camera> Cameras; 
    Cameras cameras() const;
    static QString error_name(int code);
    
  class error : public std::runtime_error {
  public:
    error(const QString &label, int code);
  };
private:
  D_PTR
};

#endif // QHYDRIVER_H
