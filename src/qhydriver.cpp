#include "qhydriver.h"
#include "qhyccd.h"
using namespace std;

class QHYDriver::Private {
public:
  Private(QHYDriver *q);
private:
  QHYDriver *q;
};


QHYDriver::Private::Private(QHYDriver* q) : q(q)
{

}


QHYDriver::QHYDriver() : dpointer(this)
{
  if(int result = InitQHYCCDResource() != QHYCCD_SUCCESS) {
    throw runtime_error("Error initializing QHYCCD Library");
  }
}

QHYDriver::~QHYDriver()
{
}


vector< QHYDriver::Camera > QHYDriver::cameras() const
{

}
