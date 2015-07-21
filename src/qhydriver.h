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
      char id;
      int index;
      QString name;
    };
    QList<Camera> cameras() const;
private:
  D_PTR
};

#endif // QHYDRIVER_H
