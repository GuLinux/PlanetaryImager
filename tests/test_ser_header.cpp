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


class SerHeaderColor : public ::testing::TestWithParam<pair<qint32, Frame::ColorFormat>> {
public:
  void SetUp() override {
    auto value = GetParam();
    ser_color_id = value.first;
    expected_color_format = value.second;
  }
  qint32 ser_color_id;
  Frame::ColorFormat expected_color_format;
};

TEST_P(SerHeaderColor, should_map_SER_color_into_frame_ColorFormat) {
  SER_Header header;
  header.colorId = ser_color_id;
  ASSERT_EQ(expected_color_format, header.frame_color_format());
}

INSTANTIATE_TEST_SUITE_P(SerHeaderColorValuesTest, SerHeaderColor, ::testing::Values(
  pair<qint32, Frame::ColorFormat>{SER_Header::MONO, Frame::Mono},
  pair<qint32, Frame::ColorFormat>{999999, Frame::Mono}, // Default value for unknown number should be Mono
  pair<qint32, Frame::ColorFormat>{SER_Header::BAYER_RGGB, Frame::Bayer_RGGB},
  pair<qint32, Frame::ColorFormat>{SER_Header::BAYER_GRBG, Frame::Bayer_GRBG},
  pair<qint32, Frame::ColorFormat>{SER_Header::BAYER_GBRG, Frame::Bayer_GBRG},
  pair<qint32, Frame::ColorFormat>{SER_Header::BAYER_BGGR, Frame::Bayer_BGGR},
  pair<qint32, Frame::ColorFormat>{SER_Header::BAYER_CYYM, Frame::Mono}, // Currently unsupported
  pair<qint32, Frame::ColorFormat>{SER_Header::BAYER_YCMY, Frame::Mono}, // Currently unsupported,
  pair<qint32, Frame::ColorFormat>{SER_Header::BAYER_YMCY, Frame::Mono}, // Currently unsupported,
  pair<qint32, Frame::ColorFormat>{SER_Header::BAYER_MYYC, Frame::Mono}, // Currently unsupported,
  pair<qint32, Frame::ColorFormat>{SER_Header::RGB, Frame::RGB},
  pair<qint32, Frame::ColorFormat>{SER_Header::BGR, Frame::BGR}
));
