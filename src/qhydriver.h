#ifndef QHYDRIVER_H
#define QHYDRIVER_H
#include "dptr.h"

#include <vector>
class QHYDriver
{
public:
    QHYDriver();
    ~QHYDriver();
    struct Camera {
      int id;
      std::string name;
    };
    std::vector<Camera> cameras() const;
private:
  D_PTR
};

#endif // QHYDRIVER_H
