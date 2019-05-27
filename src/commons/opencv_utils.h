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


#ifdef CV_VERSION_EPOCH
#if CV_VERSION_EPOCH < 3 
#warning "Using OpenCV2 compat functions"
namespace cv {
//    const int IMREAD_COLOR = CV_LOAD_IMAGE_COLOR;
//    const int COLOR_BGR2GRAY = CV_BGR2GRAY;
//    const int COLOR_RGB2HSV = CV_RGB2HSV;
//    const int COLOR_BayerRG2BGR = CV_BayerRG2BGR;
//    const int COLOR_BayerGB2BGR = CV_BayerGB2BGR;
//    const int COLOR_BayerGR2BGR = CV_BayerGR2BGR;
//    const int COLOR_BayerBG2BGR = COLOR_BayerBG2BGR;
//    const int COLOR_BGR2RGB = CV_BGR2RGB;
//    const int COLOR_GRAY2RGB = CV_GRAY2RGB;
//    const int COLOR_RGB2GRAY = CV_RGB2GRAY;
//    const int COLOR_RGBA2RGB = CV_RGBA2RGB;
//    const int COLOR_BGR2HSV = COLOR_BGR2HSV;
//    const int COLOR_RGBA2RGB = CV_RGBA2RGB;
//    const int COLOR_RGBA2RGB = CV_RGBA2RGB;
//    const int COLOR_HSV2BGR = CV_HSV2BGR;
//    const int COLOR_BGR2HSV = CV_BGR2HSV;
//    const int IMWRITE_JPEG_QUALITY = CV_IMWRITE_JPEG_QUALITY;
//    const int IMWRITE_PXM_BINARY = CV_IMWRITE_PXM_BINARY;
//    const int IMREAD_UNCHANGED = CV_LOAD_IMAGE_UNCHANGED;

namespace Error {
    const int StsUnsupportedFormat = CV_StsUnsupportedFormat;
}
#define CV_FOURCC_COMPAT CV_FOURCC
}
#else
#define CV_FOURCC_COMPAT(a,b,c,d) cv::VideoWriter::fourcc(a,b,c,d)
#endif
#endif

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
