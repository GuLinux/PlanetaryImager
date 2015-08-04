#include "fps_counter.h"
#include <QTimer>
#include <QElapsedTimer>
using namespace std;

class fps_counter::Private
{
public:
  Private ( const OnFPS &onfps, Mode mode, int fps_trigger_milliseconds, fps_counter *q );
  OnFPS onfps;
  Mode mode;
  int fps_trigger_milliseconds;
  QElapsedTimer count_time;
  uint64_t frames = 0;
  void timeout();

private:
  fps_counter *q;
};

fps_counter::Private::Private ( const OnFPS& onfps, fps_counter::Mode mode, int fps_trigger_milliseconds, fps_counter* q )
  : onfps{onfps}, mode{mode}, fps_trigger_milliseconds{fps_trigger_milliseconds}, q{q}
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

fps_counter::fps_counter ( const OnFPS& onFPS, fps_counter::Mode mode, int fps_trigger_milliseconds, QObject* parent ) 
  : QObject ( parent ), dpointer ( onFPS, mode, fps_trigger_milliseconds, this )
{
  if(mode == Timer) {
    QTimer *timer = new QTimer{this};
    connect(timer, &QTimer::timeout, bind(&Private::timeout, d.get()));
    timer->start(fps_trigger_milliseconds);
  }
  d->count_time.start();
}


void fps_counter::frame()
{
  ++d->frames;
  if(d->mode == Elapsed && d->count_time.elapsed() >= d->fps_trigger_milliseconds)
    d->timeout();
}


#include "fps_counter.moc"