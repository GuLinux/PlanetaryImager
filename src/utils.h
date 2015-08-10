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

#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QApplication>
template<typename T>
void future_run(const std::function<T()> &runnable, std::function<void(QFuture<T>)> onFinished) {
  QFutureWatcher< T > *watcher = new QFutureWatcher<T>();
  QObject::connect(watcher, &QFutureWatcher<T>::finished, [=]{onFinished(watcher->future()); });
  QObject::connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
  QObject::connect(watcher, SIGNAL(canceled()), watcher, SLOT(deleteLater()));
  watcher->setFuture(QtConcurrent::run(runnable));
}

#define print_thread_id \
  static bool printed = false; \
  if(!printed) { \
    qDebug() << __PRETTY_FUNCTION__ << ": thread_id=" << QThread::currentThreadId(); \
    printed = true; \
  }


#endif