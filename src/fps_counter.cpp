#include "fps_counter.h"
#include <QTimer>
#include <QElapsedTimer>
using namespace std;

class fps_counter::Private
{
public:
  Private ( const OnFPS &onfps, fps_counter *q );
  OnFPS onfps;
  QElapsedTimer count_time;
  uint64_t frames = 0;
  void timeout();

private:
  fps_counter *q;
};

fps_counter::Private::Private ( const OnFPS& onfps, fps_counter* q ) : onfps(onfps), q ( q )
{
}

fps_counter::~fps_counter()
{
}

void fps_counter::Private::timeout()
{
  double fps = static_cast<double>(frames) * 1000/(static_cast<double>(count_time.elapsed()));
  onfps(fps);
  count_time.restart();
  frames = 0;
}



fps_counter::fps_counter ( const OnFPS &onFPS, int fps_trigger_milliseconds, QObject* parent )
  : dpointer ( onFPS, this )
{
  QTimer *timer = new QTimer{this};
  connect(timer, &QTimer::timeout, bind(&Private::timeout, d.get()));
  timer->start(fps_trigger_milliseconds);
  d->count_time.start();
}

void fps_counter::frame()
{
  ++d->frames;
}


#include "fps_counter.moc"