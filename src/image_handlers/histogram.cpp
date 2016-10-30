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

#include <QtConcurrent/QtConcurrent>
#include "commons/configuration.h"
#include <atomic>
#include "c++/stringbuilder.h"

#include <opencv2/opencv.hpp>
#include <QImage>
using namespace std;

DPTR_IMPL(Histogram) {
  Configuration &configuration;
  Histogram *q;
  atomic_bool recording;
  atomic_bool histogram_disable_on_recording;
  atomic_long histogram_timeout;
  atomic_long histogram_timeout_recording;
  QElapsedTimer last;
  size_t bins_size;
  bool should_read_frame() const;
  bool logarithmic = false;
};


Histogram::~Histogram()
{
}

Histogram::Histogram(Configuration &configuration, QObject* parent) : QObject(parent), dptr(configuration, this)
{
  d->last.start();
  d->recording = false;
  read_settings();
}

#include <fstream>
void Histogram::handle(const Frame::ptr &frame)
{
  
  if( ! d->should_read_frame() )
    return;
  d->last.restart();
  cv::Mat source;
  frame->mat().copyTo(source);
  if(frame->channels() > 1)
    cv::cvtColor(source,  source, cv::COLOR_BGR2GRAY);
  
  cv::Mat hist;
  int nimages = 1;
  int channels[]{0};
  int dims = 1;
  int bins []{static_cast<int>(d->bins_size)};
  float range[]{0, static_cast<float>( frame->bpp() == 8 ? numeric_limits<uint8_t>().max() : numeric_limits<uint16_t>().max() ) };
  const float *ranges[]{range};
  
  cv::calcHist(&source, nimages, channels, cv::Mat{}, hist, dims, bins, ranges);
  if(d->logarithmic)
    transform(hist.begin<float>(), hist.end<float>(), hist.begin<float>(), [](float n){ return n==0?0:log10(n); });
  
  int hist_w = 1024; int hist_h = 600;
  int bin_w = cvRound( (double) hist_w/d->bins_size );
  
  cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

  normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat() );
  /// Draw for each channel
  for(int i = 1; i <= d->bins_size; i++)
  cv::line( histImage, cv::Point(bin_w*(i), 0), cv::Point(bin_w*(i), hist_h), cv::Scalar(50, 50, 50), 1, CV_AA);
  for( int i = 1; i < d->bins_size; i++ )
  {
    cv::line( histImage, cv::Point( bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ) ,
              cv::Point( bin_w*(i), hist_h - cvRound(hist.at<float>(i)) ),
          cv::Scalar( 255, 255, 255), 1, 8, 0  );
  }
  QImage hist_qimage(histImage.data, hist_w, hist_h, static_cast<int>(histImage.step), QImage::Format_RGB888);
  emit histogram(hist_qimage.rgbSwapped());
}


void Histogram::set_bins(size_t bins_size)
{
  d->bins_size = bins_size;
}

void Histogram::setRecording(bool recording)
{
  d->recording = recording;
}

void Histogram::read_settings()
{
  d->histogram_disable_on_recording = d->configuration.histogram_disable_on_recording();
  d->histogram_timeout = d->configuration.histogram_timeout();
  d->histogram_timeout_recording = d->configuration.histogram_timeout_recording();
}


bool Histogram::Private::should_read_frame() const // TODO read limits only once, when changed
{
  if( recording && histogram_disable_on_recording  )
    return false;
  return last.elapsed() >= (recording ? histogram_timeout_recording : histogram_timeout );
}

void Histogram::setLogarithmic(bool logarithmic)
{
  d->logarithmic = logarithmic;
}



#include "histogram.moc"
