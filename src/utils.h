#ifndef UTILS_H
#define UTILS_H
#include <QString>

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


#endif