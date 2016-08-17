/*
 * Copyright (C) 2016  Marco Gulino <marco@gulinux.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
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
