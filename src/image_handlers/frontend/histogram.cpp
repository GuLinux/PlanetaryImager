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
#include <cmath>
#include "c++/stlutils.h"
#include <QColor>
#include "histLib.h"

using namespace std;

DPTR_IMPL(Histogram) {
  Configuration::ptr configuration;
  Histogram *q;
  atomic_bool recording;
  atomic_bool histogram_disable_on_recording;
  atomic_long histogram_timeout;
  atomic_long histogram_timeout_recording;
  QElapsedTimer last;
  size_t bins_size;
  bool should_read_frame() const;
  bool logarithmic = false;
  Channel channel = Grayscale;
  Frame::ptr last_frame;
  void handle(const Frame::ptr &frame);
  struct HistogramOutput {
    cv::Mat histogram;
    QVariantMap stats;
  };
  HistogramOutput calcHistogram(const cv::Mat &source, int bpp);
  HistogramOutput calcHistogramStats(const cv::Mat &source, cv::Mat &histogram, int bpp);
  void drawHistogram(const cv::Mat &histogram, int bins_width, cv::Mat &image, const QColor &color = Qt::white);
  
  CHistLib histLib;
};


Histogram::~Histogram()
{
}

Histogram::Histogram(const Configuration::ptr &configuration, QObject* parent) : QObject(parent), dptr(configuration, this)
{
  d->last.start();
  d->recording = false;
  read_settings();
  static bool metatypes_registered = false;
  if(!metatypes_registered) {
    metatypes_registered = true;
    qRegisterMetaType<Histogram::Channel>("Histogram::Channel");
    qRegisterMetaType<QMap<Histogram::Channel, QVariantMap>>("QMap<Histogram::Channel, QVariantMap>");
  }
}

void Histogram::handle(const Frame::ptr &frame)
{
  if( ! d->should_read_frame() )
    return;
  d->last.restart();
  d->handle(frame);
  d->last_frame = frame;
}

void Histogram::Private::handle(const Frame::ptr& frame)
{
  BENCH(_histogram)->every(5)->ms();
  cv::Mat source;
  
  static const  int hist_h = 600;
  histLib.SetHistImageHeight(hist_h);
  histLib.SetDrawSpreadOut(true);
  
  cv::Mat plot;
  QMap<Histogram::Channel, QVariantMap> histogramStats;
  
  frame->mat().copyTo(source);
  if(frame->channels() == 1) {
    channel = Grayscale;
  }
  if(channel != Grayscale && channel != All) {
    static map<Frame::ColorFormat, map<Channel, int>> channel_indexes {
      {Frame::RGB, { {Red, 0}, {Green, 1 }, {Blue, 2} }},
      {Frame::BGR, { {Red, 2}, {Green, 1 }, {Blue, 0} }},
    };
    vector<cv::Mat> channels(3);
    cv::split(source, channels);
    source = channels[ channel_indexes[frame->colorFormat()][channel] ];
  }
  
  if(channel !=  All) {
    static QMap<Channel, cv::Scalar> colors {
      {Grayscale, HIST_LIB_COLOR_WHITE},
      {Red, HIST_LIB_COLOR_RED},
      {Green, HIST_LIB_COLOR_GREEN},
      {Blue, HIST_LIB_COLOR_BLUE},
    };
    histLib.SetPlotColor(colors[channel]);
    cv::MatND hist;
    histLib.ComputeHistogramValue(source, hist);
    
    HistogramOutput out = calcHistogramStats(source, hist, frame->bpp());
    histogramStats[channel] = out.stats;

    histLib.DrawHistogramValue(out.histogram, plot);
  }
  else {
    cv::MatND histB, histR, histG;
    histLib.ComputeHistogramBGR(source, histB, histG, histR);
    
    histogramStats[Red]  = calcHistogramStats(source, histR, frame->bpp()).stats;
    histogramStats[Green]  = calcHistogramStats(source, histG, frame->bpp()).stats;
    histogramStats[Blue]  = calcHistogramStats(source, histB, frame->bpp()).stats;
    
    histLib.DrawHistogramBGR(histB, histG, histR, plot);
  }
  QImage hist_qimage(plot.data, plot.cols, plot.rows, static_cast<int>(plot.step), QImage::Format_RGB888);
  emit q->histogram(hist_qimage.rgbSwapped(), histogramStats, channel);
}


Histogram::Private::HistogramOutput Histogram::Private::calcHistogram(const cv::Mat& source, int bpp)
{
  cv::Mat hist;
  int nimages = 1;
  int channels[]{0};
  int dims = 1;
  int bins []{static_cast<int>(bins_size)};
  float range[]{0, static_cast<float>( pow(2, bpp)-1 ) };
  const float *ranges[]{range};
  
  cv::calcHist(&source, nimages, channels, cv::Mat{}, hist, dims, bins, ranges);
  return calcHistogramStats(source, hist, bpp);
}

Histogram::Private::HistogramOutput Histogram::Private::calcHistogramStats(const cv::Mat& source, cv::Mat& histogram, int bpp)
{
  auto maxBin =  pow(2, bpp)-1;
  auto top_bin_it = max_element(histogram.begin<float>(), histogram.end<float>());
  auto top_bin_position = top_bin_it - histogram.begin<float>();
  auto top_bin_value_min = maxBin/bins_size * top_bin_position;
  auto top_bin_value_max = top_bin_value_min + (maxBin/bins_size);
  
  if(logarithmic)
    transform(histogram.begin<float>(), histogram.end<float>(), histogram.begin<float>(), [](float n){ return n==0?0:log10(n); });
  
  
  vector<uint16_t> mat_data;
  source.reshape(1, 1).copyTo(mat_data);
  uint16_t min_value = *min_element(begin(mat_data), end(mat_data));
  uint16_t max_value = *max_element(begin(mat_data), end(mat_data));
  
  
  QVariantMap stats{
    {"shadows_value", min_value},
    {"shadows_count", static_cast<int>(count(begin(mat_data), end(mat_data), min_value))},
    {"highlights_value", max_value},
    {"highlights_count", static_cast<int>(count(begin(mat_data), end(mat_data), max_value))},
    {"maximum_value_min", top_bin_value_min},
    {"maximum_value_max", top_bin_value_max},
    {"maximum_value_percent", 100. * top_bin_position / bins_size},
    {"range_min", 0},
    {"range_max", maxBin},
    {"pixels", source.rows * source.cols},
  };
  return {histogram, stats};
}


void Histogram::set_bins(size_t bins_size)
{
  d->bins_size = bins_size;
  d->histLib.SetBinCount(bins_size);
}

void Histogram::setRecording(bool recording)
{
  d->recording = recording;
}

void Histogram::read_settings()
{
  d->histogram_disable_on_recording = d->configuration->histogram_disable_on_recording();
  d->histogram_timeout = d->configuration->histogram_timeout();
  d->histogram_timeout_recording = d->configuration->histogram_timeout_recording();
}


bool Histogram::Private::should_read_frame() const // TODO read limits only once, when changed
{
  if( recording && histogram_disable_on_recording  )
    return false;
  return !last_frame || last.elapsed() >= (recording ? histogram_timeout_recording : histogram_timeout );
}

void Histogram::setLogarithmic(bool logarithmic)
{
  d->logarithmic = logarithmic;
}

Histogram::Channel Histogram::channel() const
{
  return d->channel;
}

void Histogram::setChannel(Histogram::Channel channel)
{
  d->channel = channel;
  if(d->last_frame)
    d->handle(d->last_frame);
}


#include "histogram.moc"
