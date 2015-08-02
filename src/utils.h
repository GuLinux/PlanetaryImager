#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QDebug>
#ifdef IN_IDE_PARSER
#define _q + QString()
#else
inline QString operator ""_q(const char *s, std::size_t) { return QString{s}; }
#endif

template<typename T>
QString operator%(const QString &other, const T &t) {
  return other.arg(t);
}


template<>
inline QString operator%(const QString &first, const std::string &second) {
  return first.arg(QString::fromStdString(second));
}


inline QDebug operator<<(QDebug dbg, const std::string &str) {
  dbg << QString::fromStdString(str);
  return dbg;
}


#include <QElapsedTimer>
#include <functional>
class benchmarck {
public:
  typedef std::function<void(const QString &, int, double)> BenchmarkCall;
  benchmarck(const QString &name, BenchmarkCall benchmark_f = benchmarck::debug_benchmark(), int print_every = 20) 
    : name(name), print_every(print_every), benchmark_f(benchmark_f) { timer.start(); }
  ~benchmarck() {
    static QMap<QString, QVector<double>> timers;
    timers[name].push_back(timer.elapsed());
    if(timers[name].size() >= print_every) {
      benchmark_f(name, timers[name].size(), std::accumulate(std::begin(timers[name]), std::end(timers[name]), 0.)/timers[name].size());
      timers[name].clear();
    }
  }
private:
  const QString name;
  const int print_every;
  BenchmarkCall benchmark_f;
  QElapsedTimer timer;
  static BenchmarkCall debug_benchmark() { return [](const QString &name, int elements, double elapsed){ qDebug() << "benchmark for" << name << "(avg):"<< elapsed << "ms"; }; }
};

class fps {
public:
  typedef std::function<void(double fps)> OnFPS;
  fps(OnFPS onFPS, int64_t fps_after_msec = 500) : onFPS(onFPS), fps_after_msec(fps_after_msec), frames{0} { timer.start(); }
  void add_frame() {
    frames++;
    if(timer.elapsed() >= fps_after_msec) {
      onFPS(static_cast<double>(frames) * 1000/(static_cast<double>(timer.elapsed())));
      timer.restart();
      frames = 0;
    }
  }
private:
  OnFPS onFPS;
  int64_t fps_after_msec;
  int64_t frames;
  QElapsedTimer timer;
};

#endif