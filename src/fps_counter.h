#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

#include <QObject>
#include "dptr.h"

class fps_counter : public QObject
{
  Q_OBJECT
public:
  typedef std::function<void(double fps)> OnFPS;
  enum Mode { Timer, Elapsed };
  ~fps_counter();
  fps_counter(const OnFPS& onFPS, Mode mode = Timer, int fps_trigger_milliseconds = 1000, bool mean = false, QObject* parent = 0);
  fps_counter &operator++();

private:
    DPTR
};

#endif // FPS_COUNTER_H
