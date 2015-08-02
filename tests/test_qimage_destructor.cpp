#include <QTest>
#include <QImage>
#include <QDebug>
#include <memory>

class TestQImageDestructor : public QObject {
  Q_OBJECT
private slots:
  void init();
  void testPlainStruct();
  void testSharedPointerStruct();
private:
  bool destroyed;
  uchar image_data[4];
};

using namespace std;

struct Foo {
  Foo(bool &destroyed) : destroyed(destroyed){}
  bool &destroyed;
  ~Foo() { destroyed = true; }
};

void TestQImageDestructor::init()
{
  destroyed = false;
}


void TestQImageDestructor::testPlainStruct()
{
  Foo *foo = new Foo{destroyed};
  {
    QImage image(image_data, 2, 2, QImage::Format_Grayscale8, [](void *mem){ Foo *foo = (Foo*) mem; delete foo; }, foo);
    QVERIFY(!image.isNull());
  }
  QVERIFY(destroyed);
}

void TestQImageDestructor::testSharedPointerStruct()
{
  {
    auto foo = make_shared<Foo>(destroyed);
    auto bla = new shared_ptr<Foo>(foo);
    {
      QImage image(image_data, 2, 2, QImage::Format_Grayscale8, [](void *mem){ shared_ptr<Foo> *foo = (shared_ptr<Foo>*) mem; delete foo; }, bla);
      QVERIFY(!image.isNull());
    }
    QVERIFY(!destroyed);
  }
  QVERIFY(destroyed);
}


QTEST_GUILESS_MAIN(TestQImageDestructor);
#include "test_qimage_destructor.moc"