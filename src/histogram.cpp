/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#include "histogram.h"
#include <functional>
#include <opencv2/opencv.hpp>
using namespace std;

class Histogram::Private {
public:
  Private(Histogram *q);
  QElapsedTimer last;
private:
  Histogram *q;
};

Histogram::Private::Private(Histogram* q) : q{q}
{
}


Histogram::~Histogram()
{
}

Histogram::Histogram(QObject* parent) : QObject(parent), dptr(this)
{
  d->last.start();
}

void Histogram::handle(const cv::Mat& imageData)
{
  if(d->last.elapsed() < 1000)
    return;
  cv::Mat hist;
  cv::Mat gray;
  typedef function<cv::Mat(const cv::Mat &)> tr_img;
  if(imageData.channels() == 3) {
    cv::cvtColor(imageData, gray, CV_BGR2GRAY);
  } else {
    gray = imageData;
  }
  int histSize = 256;
  float range[] = { 0, imageData.depth() == CV_8U ? 256 : 256*256 } ;
  const float* histRange = { range };
  const int channels = imageData.channels();
  cv::calcHist( &gray, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, true, false);
//   cv::normalize(hist, hist, 0., 256, cv::NORM_MINMAX);
  emit histogram(hist);
  d->last.restart();
}

#include "histogram.moc"