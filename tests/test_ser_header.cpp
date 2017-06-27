#include "gtest/gtest.h"
#include "commons/ser_header.h"
#include <QDateTime>
#include <QDebug>
using namespace std;


TEST(SerHeader, test_qdatetime_utc_to_ser) {
  QDateTime now{{2017, 06, 20}, {10, 10, 00}, Qt::UTC};
  auto ser_datetime = SER_Header::timestamp(now);
  ASSERT_EQ(636335502000000000, ser_datetime);
}


TEST(SerHeader, test_qdatetime_local_to_ser) {
  QDateTime now{{2017, 06, 20}, {10, 10, 00}, Qt::LocalTime, 3600};
  auto ser_datetime = SER_Header::timestamp(now);
  ASSERT_EQ(636335502000000000, ser_datetime);
}

