#ifndef UTILS_H
#define UTILS_H

#include <QVector>
#include <QElapsedTimer>
#include <functional>
#include <QDebug>

#define dbg_print_thread_id \
  static bool printed = false; \
  if(!printed) { \
    qDebug() << __PRETTY_FUNCTION__ << ": thread_id=" << QThread::currentThreadId(); \
    printed = true; \
  }

  
#endif