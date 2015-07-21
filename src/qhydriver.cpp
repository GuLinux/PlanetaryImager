#include "qhydriver.h"
#include "qhyccd.h"
#include <map>
#include "utils.h"
#include <QDebug>
#include <QString>
using namespace std;

class QHYDriver::Private {
public:
  Private(QHYDriver *q);
  static map<int,QString> error_codes;
  static map<char,QString> device_codes;
  class error : public std::runtime_error {
  public:
    error(const QString &label, int code) : runtime_error(("Error on %1: %2 (%3)"_q % label % code % error_codes[code]).toLatin1()) {}
  };
private:
  QHYDriver *q;
};

#include "qhy_messages.cpp"

QHYDriver::Private::Private(QHYDriver* q) : q(q)
{

}


QHYDriver::QHYDriver() : dpointer(this)
{
  if(int result = InitQHYCCDResource() != QHYCCD_SUCCESS)
    throw Private::error("initializing QHY Driver", result);
}

QHYDriver::~QHYDriver()
{
  if(int result = ReleaseQHYCCDResource() != QHYCCD_SUCCESS)
    throw Private::error("releasing QHY Driver", result);
}


QList< QHYDriver::Camera > QHYDriver::cameras() const
{
  QList<Camera> cameras;
  int scan_result= ScanQHYCCD();
  if(scan_result == QHYCCD_ERROR_NO_DEVICE) {
    return cameras;
  }
  if(scan_result < QHYCCD_SUCCESS) {
    throw Private::error{"getting QHY cameras list", scan_result};
  }
  for(int i=0; i<scan_result; i++) {
    char id = 0;
    if(int result = GetQHYCCDId(i, &id) != QHYCCD_SUCCESS)
      throw Private::error{"getting QHY Camera ID", result};
    cameras.push_back({id, i, Private::device_codes[id]});
  }
  return cameras;
}
