#ifndef UTILS_H
#define UTILS_H

#include <QVector>
#include <QElapsedTimer>
#include <functional>
#include <QDebug>
#include <QString>

#define dbg_print_thread_id \
  static bool printed = false; \
  if(!printed) { \
    qDebug() << __PRETTY_FUNCTION__ << ": thread_id=" << QThread::currentThreadId(); \
    printed = true; \
  }

class LogScope {
public:
  LogScope(const QString &fname, const QString &enter = "ENTER", const QString &exit = "EXIT") : fname{fname}, exit{exit} {
    qDebug() << enter << " " << fname;
  }
  ~LogScope() {
    qDebug() << exit << " " << fname;
  }
private:
  const QString fname;
  const QString exit;
};

#define LOG_F_SCOPE LogScope log_current_scope(__PRETTY_FUNCTION__);
#define LOG_C_SCOPE log_current_class( typeid(*this).name(), "Create Object", "Delete Object" )
  
#endif