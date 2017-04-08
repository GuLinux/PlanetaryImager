#include "gtest/gtest.h"
#include "drivers/roi.h"
#include <QString>
#include <QDebug>
using namespace std;

ostream &operator<<(ostream &o, const QRect &r) {
  QString s;
  QDebug d(&s);
  d << r;
  return o << s.toStdString();
}


TEST(ROIValidatorTest, test_max_resolution) {
  auto rect = QRect{0, 0, 1080, 1080};
  ROIValidator::max_resolution(QRect{0, 0, 1024, 768})(rect);
  ASSERT_EQ( (QRect{0, 0, 1024,768}), rect);
}


TEST(ROIValidatorTest, test_width_multiple) {
  auto rect = QRect{0, 0, 1089, 1080};
  ROIValidator::width_multiple(4)(rect);
  ASSERT_EQ( (QRect{0, 0, 1088,1080}), rect);
  rect.setWidth(1087);
  ROIValidator::width_multiple(4)(rect);
  ASSERT_EQ( (QRect{0, 0, 1084,1080}), rect);
}


TEST(ROIValidatorTest, test_height_multiple) {
  auto rect = QRect{0, 0, 1089, 555};
  ROIValidator::height_multiple(8)(rect);
  ASSERT_EQ( (QRect{0, 0, 1089,552}), rect);
  rect.setHeight(1087);
  ROIValidator::height_multiple(8)(rect);
  ASSERT_EQ( (QRect{0, 0, 1089,1080}), rect);
}


TEST(ROIValidatorTest, test_hflip) {
  QRect rect, context;
  rect.setCoords(1, 0, 7, 9);
  context.setCoords(0, 0, 9, 9);
  auto width = rect.width();
  rect = ROIValidator::flipped(rect, true, false, QRect{0, 0, 10, 10});
  ASSERT_EQ( (QRect{2, 0, 7,10}), rect);
  ASSERT_EQ( width, rect.width());
  ASSERT_EQ(8, rect.bottomRight().x());
}




TEST(ROIValidatorTest, test_vflip) {
  QRect rect, context;
  rect.setCoords(0, 3, 9, 8);
  context.setCoords(0, 0, 9, 9);
  auto height = rect.height();
  rect = ROIValidator::flipped(rect, false, true, QRect{0, 0, 10, 10});
  ASSERT_EQ( (QRect{0, 1, 10,6}), rect);
  ASSERT_EQ( height, rect.height());
  ASSERT_EQ(6, rect.bottomRight().y());
}

TEST(ROIValidatorTest, sequence_test) {
  auto rect = QRect{10, 20, 1245, 421};
  ROIValidator validator{{ROIValidator::width_multiple(8), ROIValidator::height_multiple(2)}};
  auto new_rect = validator.validate(rect, QRect{});
  ASSERT_EQ(0, new_rect.width() % 8);
  ASSERT_EQ(0, new_rect.height() % 2);
  ASSERT_EQ(10, new_rect.x());
  ASSERT_EQ(20, new_rect.y());
}


TEST(ROIValidatorTest, roi_in_roi_test) {
  auto rect = QRect{10, 20, 1245, 421};
  auto currentROI = QRect{30, 45, 2203, 4432};
  ROIValidator validator{{}};
  auto new_rect = validator.validate(rect, currentROI);
  ASSERT_EQ(40, new_rect.x());
  ASSERT_EQ(65, new_rect.y());
  ASSERT_EQ(1245, new_rect.width());
  ASSERT_EQ(421, new_rect.height());
}

TEST(ROIValidatorTest, area_test) {
  auto rect = QRect{10, 20, 4242, 2422};
  ROIValidator::area_multiple(1024, 0, 2)(rect);
  ASSERT_EQ(0, rect.height() % 2);
  ASSERT_EQ(0, (rect.height() * rect.width()) % 1024);
  qDebug() << rect;
  rect = QRect{10, 20, 1842, 1422};
  ROIValidator::area_multiple(1024, 2, 0)(rect);
  ASSERT_EQ(0, rect.width() % 2);
  ASSERT_EQ(0, (rect.height() * rect.width()) % 1024);
  qDebug() << rect;
  
 rect = QRect{10, 20, 1842, 1422};
  ROIValidator::area_multiple(1024, 4, 0)(rect);
  ASSERT_EQ(QRect{}, rect);
  qDebug() << rect;
}
