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

#include "histogram.h"
#include <functional>
#include <opencv2/opencv.hpp>

#define cimg_display 0
#define cimg_plugin "plugins/cvMat.h"
#include <CImg.h>
using namespace cimg_library;

using namespace std;

DPTR_IMPL(Histogram) {
  Histogram *q;
  QElapsedTimer last;
  size_t bins_size;
};


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

  CImg<uint32_t> image(imageData);
  image.histogram(d->bins_size);
  vector<uint32_t> hist(image.size());
  move(image.begin(), image.end(), hist.begin());
  
  emit histogram(hist);
  d->last.restart();
}


void Histogram::set_bins(size_t bins_size)
{
  d->bins_size = bins_size;
}

#include "histogram.moc"
