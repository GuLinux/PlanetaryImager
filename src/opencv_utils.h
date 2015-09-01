#ifndef OPENCV_UTILS_H
#define OPENCV_UTILS_H
#include <opencv2/opencv.hpp>
#include <QDebug>

inline QDebug operator<<(QDebug dbg, const cv::Point &point) {
  dbg.nospace() << "{" << point.x << "," << point.y << "}";
  return dbg.space();
}

inline QDebug operator<<(QDebug dbg, const cv::Size &size) {
  dbg.nospace() << "cv::Size{" << size.width << "," << size.height << "}";
  return dbg.space();
}

inline QDebug operator<<(QDebug dbg, const cv::Rect &rect) {
  dbg.nospace() << "cv::Rect{" << rect.tl() << "x" << rect.br() << "," << rect.size() << "}";
  return dbg.space();
}
inline QDebug operator<<(QDebug dbg, const cv::Range &range) {
  dbg.nospace() << "cv::Range{" << range.start << ", " << range.end << "}";
  return dbg.space();
}
inline QDebug operator<<(QDebug dbg, const cv::Mat &mat) {
  dbg.nospace() << "cv::Mat{" << mat.cols << "x" << mat.rows << ", type=" 
    << mat.type() << ", depth=" << mat.depth() << ", channels=" << mat.channels()
    << ", valid=" << (mat.data == nullptr ? "false" : "true");
  return dbg.space();
}
#endif
