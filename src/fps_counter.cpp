#include "fps_counter.h"
#include <QDebug>
#include <QTimer>
#include <QElapsedTimer>
using namespace std;

class fps_counter::Private
{
public:
  Private ( const OnFPS &onfps, Mode mode, int fps_trigger_milliseconds, bool mean, fps_counter *q );
  OnFPS onfps;
  Mode mode;
  int fps_trigger_milliseconds;
  QElapsedTimer count_time;
  QElapsedTimer total_time;
  uint64_t frames = 0;
  void timeout();
  bool mean;

private:
  fps_counter *q;
};

fps_counter::Private::Private ( const OnFPS& onfps, fps_counter::Mode mode, int fps_trigger_milliseconds, bool mean, fps_counter* q )
  : onfps{onfps}, mode{mode}, fps_trigger_milliseconds{fps_trigger_milliseconds}, mean{mean}, q{q}
{
}

fps_counter::~fps_counter()
{
}

void fps_counter::Private::timeout()
{
  double elapsed = static_cast<double>(mean ? total_time.elapsed() : count_time.elapsed());
  double fps = static_cast<double>(frames) * 1000/(elapsed);
  onfps(fps);
  count_time.restart();
  if(!mean) {
    frames = 0;
  }
}

fps_counter::fps_counter ( const OnFPS& onFPS, fps_counter::Mode mode, int fps_trigger_milliseconds, bool mean, QObject* parent ) 
  : QObject ( parent ), dptr ( onFPS, mode, fps_trigger_milliseconds, mean, this )
{
  if(mode == Timer) {
    QTimer *timer = new QTimer{this};
    connect(timer, &QTimer::timeout, bind(&Private::timeout, d.get()));
    timer->start(fps_trigger_milliseconds);
  }
  d->count_time.start();
  if(mean)
    d->total_time.start();
}


fps_counter& fps_counter::operator++()
{
  ++d->frames;
  if(d->mode == Elapsed && d->count_time.elapsed() >= d->fps_trigger_milliseconds)
    d->timeout();
  return *this;
}



#include "fps_counter.moc"