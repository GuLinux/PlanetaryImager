#include "gtest/gtest.h"
#include "drivers/roi.h"
#include <QString>
#include <QDebug>
using namespace std;

struct ROIValidatorTest : public ::testing::Test {
  QRect rect;
};

ostream &operator<<(ostream &o, const QRect &r) {
  QString s;
  QDebug d(&s);
  d << r;
  return o << s.toStdString();
}


TEST_F(ROIValidatorTest, test_max_resolution) {
  rect = QRect{0, 0, 1080, 1080};
  ROIValidator::max_resolution(QRect{0, 0, 1024, 768})(rect);
  ASSERT_EQ( (QRect{0, 0, 1024,768}), rect);
}


TEST_F(ROIValidatorTest, test_width_multiple) {
  rect = QRect{0, 0, 1089, 1080};
  ROIValidator::width_multiple(4)(rect);
  ASSERT_EQ( (QRect{0, 0, 1088,1080}), rect);
  rect.setWidth(1087);
  ROIValidator::width_multiple(4)(rect);
  ASSERT_EQ( (QRect{0, 0, 1084,1080}), rect);
}


TEST_F(ROIValidatorTest, test_height_multiple) {
  rect = QRect{0, 0, 1089, 555};
  ROIValidator::height_multiple(8)(rect);
  ASSERT_EQ( (QRect{0, 0, 1089,552}), rect);
  rect.setHeight(1087);
  ROIValidator::height_multiple(8)(rect);
  ASSERT_EQ( (QRect{0, 0, 1089,1080}), rect);
}

TEST_F(ROIValidatorTest, sequence_test) {
  rect = QRect{10, 20, 1245, 421};
  ROIValidator validator{ROIValidator::width_multiple(8), ROIValidator::height_multiple(2)};
  auto new_rect = validator.validate(rect);
  ASSERT_EQ(0, new_rect.width() % 8);
  ASSERT_EQ(0, new_rect.height() % 2);
  ASSERT_EQ(10, new_rect.x());
  ASSERT_EQ(20, new_rect.y());
}
