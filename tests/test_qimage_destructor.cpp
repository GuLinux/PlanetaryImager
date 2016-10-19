#include "gtest/gtest.h"
#include <QImage>
#include <QDebug>
#include <memory>

class TestQImageDestructor : public ::testing::Test {
public:
  bool destroyed = false;
  uchar image_data[4*4];
};

using namespace std;

struct Foo {
  Foo(bool &destroyed) : destroyed(destroyed){}
  bool &destroyed;
  ~Foo() { destroyed = true; }
};


TEST_F(TestQImageDestructor, testPlainStruct)
{
  Foo *foo = new Foo{destroyed};
  {
    QImage image(image_data, 2, 2, QImage::Format_RGBX8888, [](void *mem){ Foo *foo = (Foo*) mem; delete foo; }, foo);
    ASSERT_FALSE(image.isNull());
  }
  ASSERT_TRUE(destroyed);
}

TEST_F(TestQImageDestructor, testSharedPointerStruct)
{
  {
    auto foo = make_shared<Foo>(destroyed);
    auto bla = new shared_ptr<Foo>(foo);
    {
      QImage image(image_data, 2, 2, QImage::Format_RGBX8888, [](void *mem){ shared_ptr<Foo> *foo = (shared_ptr<Foo>*) mem; delete foo; }, bla);
      ASSERT_FALSE(image.isNull());
    }
    ASSERT_FALSE(destroyed);
  }
  ASSERT_TRUE(destroyed);
}
