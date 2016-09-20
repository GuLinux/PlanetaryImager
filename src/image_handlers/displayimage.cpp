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

#include "displayimage.h"
#include "commons/configuration.h"
#include "commons/fps_counter.h"
#include <QThread>
#include <QElapsedTimer>
#include <QRect>
#include <QImage>
#include <boost/lockfree/spsc_queue.hpp>
#include <QDebug>
#include "Qt/strings.h"
#include "commons/utils.h"
#include <opencv2/opencv.hpp>
#include "commons/opencv_utils.h"
#include "c++/stlutils.h"
#include <atomic>
#include "Qt/benchmark.h"
#include <atomic>
#include "commons/utils.h"

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(DisplayImage) {
  Configuration &configuration;
  DisplayImage *q;
  unique_ptr<fps_counter> capture_fps;
  atomic_bool recording;
  atomic_bool running;
  
  // settings
  atomic_bool limit_fps;
  atomic_long min_milliseconds_elapsed;
  // settings - edge detection
  atomic_bool edge_detection_sobel;
  atomic_int sobel_blur_size;
  atomic_int sobel_kernel;
  atomic<double> sobel_scale;
  atomic<double> sobel_delta;
  
  atomic_bool edge_detection_canny;
  atomic_int canny_kernel_size;
  atomic_int canny_blur;
  atomic<double> canny_threshold_ratio;
  atomic<double> canny_low_threshold;
  
  atomic_bool debayer;
  
//         d->sobel(*cv_image, d->configuration.sobel_blur_size(), d->configuration.sobel_kernel(), d->configuration.sobel_scale(), d->configuration.sobel_delta());

  QElapsedTimer elapsed;
  QRect imageRect;
  boost::lockfree::spsc_queue<Frame::ptr, boost::lockfree::capacity<5>> queue;
  atomic_bool detectEdges;
  QVector<QRgb> grayScale;
  bool should_display_frame() const;
  void canny(cv::Mat& source, int lowThreshold = 1, int ratio = 3, int kernel_size = 3, int blurSize = 3);
  void sobel( cv::Mat& source, int blur_size = 3, int ker_size = 3, int scale = 1, int delta = 0 );
  
  void bayer2rgb(const Frame::ptr &frame, cv::Mat &image);
  void bgr2rgb(const Frame::ptr &frame, cv::Mat &image);
  void rgb2rgb(const Frame::ptr &frame, cv::Mat &image);
  void gray2rgb(const Frame::ptr &frame, cv::Mat &image);
};




DisplayImage::~DisplayImage()
{

}

DisplayImage::DisplayImage(Configuration& configuration, QObject* parent)
  : QObject(parent), dptr(configuration, this)
{
  d->capture_fps.reset(new fps_counter([=](double fps){ emit displayFPS(fps);}) );
  d->running = true;
  d->detectEdges = false;
  setRecording(false);
  for(int i=0; i<0xff; i++)
    d->grayScale.push_back(qRgb(i, i, i));
  read_settings();
  d->elapsed.restart();
}

void DisplayImage::setRecording(bool recording)
{
  d->recording = recording;
  read_settings();
}

bool DisplayImage::Private::should_display_frame() const
{
  if(!limit_fps)
    return true;
  return elapsed.elapsed() > min_milliseconds_elapsed;
}

void DisplayImage::read_settings()
{
  d->limit_fps = d->recording ? d->configuration.limit_fps_recording() : d->configuration.limit_fps();
  d->min_milliseconds_elapsed = 1000/(d->recording ? d->configuration.max_display_fps_recording() : d->configuration.max_display_fps() );
  
  d->edge_detection_sobel =  d->configuration.edge_algorithm() == Configuration::Sobel;
  d->sobel_kernel = d->configuration.sobel_kernel();
  d->sobel_blur_size = d->configuration.sobel_blur_size();
  d->sobel_delta = d->configuration.sobel_delta();
  d->sobel_scale = d->configuration.sobel_scale();

  d->edge_detection_canny =  d->configuration.edge_algorithm() == Configuration::Canny;
  d->canny_blur = d->configuration.canny_blur_size();
  d->canny_kernel_size = d->configuration.canny_kernel_size();
  d->canny_low_threshold = d->configuration.canny_low_threshold();
  d->canny_threshold_ratio = d->configuration.canny_threshold_ratio();
  d->debayer = d->configuration.debayer();
}


void DisplayImage::handle( const Frame::ptr &frame )
{
  if( ! d->should_display_frame()  || !frame->mat().data || ! d->queue.push(frame) ) {
    return;
  }
  d->elapsed.restart();
}

void DisplayImage::create_qimages()
{
  Frame::ptr frame;
  while(d->running) {
    if(!d->queue.pop(frame)) {
      QThread::msleep(1);
      continue;
    }

    ++*d->capture_fps;
    auto cv_image = new cv::Mat;
    static QHash<Frame::ColorFormat, std::function<void(const Frame::ptr &, cv::Mat&)>> converters {
      {Frame::Mono, bind(&Private::gray2rgb, d.get(), _1, _2)},
      {Frame::RGB, bind(&Private::rgb2rgb, d.get(), _1, _2)},
      {Frame::BGR, bind(&Private::bgr2rgb, d.get(), _1, _2)},
      {Frame::Bayer_RGGB, bind(&Private::bayer2rgb, d.get(), _1, _2)},
      {Frame::Bayer_GRBG, bind(&Private::bayer2rgb, d.get(), _1, _2)},
      {Frame::Bayer_GBRG, bind(&Private::bayer2rgb, d.get(), _1, _2)},
      {Frame::Bayer_BGGR, bind(&Private::bayer2rgb, d.get(), _1, _2)},
    };
    converters[frame->colorFormat()](frame, *cv_image);
    

    if(cv_image->depth() != CV_8U && cv_image->depth() != CV_8S) {
      cv_image->convertTo(*cv_image, CV_8UC3, BITS_16_TO_8);
    }
    if(d->detectEdges) {
      if(d->edge_detection_sobel) {
        QBENCH(sobel)->every(200)->ms();
        d->sobel(*cv_image, d->sobel_blur_size, d->sobel_kernel, d->sobel_scale, d->sobel_delta);
      } else if(d->edge_detection_canny) {
        QBENCH(canny)->every(200)->ms();
        d->canny(*cv_image, d->canny_low_threshold, d->canny_threshold_ratio, d->canny_kernel_size, d->canny_blur);
      }
    }
    QImage image{cv_image->data, cv_image->cols, cv_image->rows, static_cast<int>(cv_image->step), cv_image->channels() == 1 ? QImage::Format_Grayscale8: QImage::Format_RGB888, 
      [](void *data){ delete reinterpret_cast<cv::Mat*>(data); }, cv_image};
    if(cv_image->channels() == 1) {
      image.setColorTable(d->grayScale);
    }
    d->imageRect = image.rect();
    emit gotImage(image);
  }
  QThread::currentThread()->quit();
}

void DisplayImage::Private::bayer2rgb(const Frame::ptr& frame, cv::Mat& image)
{
  if(! debayer) {
    gray2rgb(frame, image);
    return;
  }
  static QHash<Frame::ColorFormat, int> bayer_patterns {
    // For some strange reason, opencv is confusing RGB and BGR
    {Frame::Bayer_RGGB, CV_BayerRG2BGR},
    {Frame::Bayer_GBRG, CV_BayerGB2BGR},
    {Frame::Bayer_GRBG, CV_BayerGR2BGR},
    {Frame::Bayer_BGGR, CV_BayerBG2BGR},
  };
  cv::cvtColor(frame->mat(), image, bayer_patterns[frame->colorFormat()]);
}

void DisplayImage::Private::bgr2rgb(const Frame::ptr& frame, cv::Mat& image)
{
  cv::cvtColor(frame->mat(), image, CV_BGR2RGB);
}

void DisplayImage::Private::gray2rgb(const Frame::ptr& frame, cv::Mat& image)
{
  cv::cvtColor(frame->mat(), image, CV_GRAY2RGB);
}

void DisplayImage::Private::rgb2rgb(const Frame::ptr& frame, cv::Mat& image)
{
  frame->mat().copyTo(image);
}


QRect DisplayImage::imageRect() const
{
  return d->imageRect;
}

void DisplayImage::quit()
{
  d->running = false;
}


void DisplayImage::detectEdges(bool detect)
{
  d->detectEdges = detect;
}

void DisplayImage::Private::canny( cv::Mat &source, int lowThreshold, int ratio, int kernel_size, int blurSize )
{
  cv::Mat src_gray, detected_edges, dst;
  cv::cvtColor( source, src_gray, CV_RGB2GRAY);
  cv::blur( src_gray, detected_edges, {blurSize,blurSize} );
  cv::Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );
  dst.create( source.size(), source.type() );
  dst = cv::Scalar::all(0);
  source.copyTo(dst, detected_edges);
  source = dst;
}


void DisplayImage::Private::sobel( cv::Mat &source, int blur_size, int ker_size, int scale, int delta )
{
  cv::Mat blurred, blurred_gray, grad;
  cv::GaussianBlur(source, blurred, {blur_size, blur_size}, 0, 0);
  cv::cvtColor( blurred, blurred_gray, CV_RGB2GRAY );
  cv::Mat grad_x, grad_y;
  cv::Mat abs_grad_x, abs_grad_y;

  /// Gradient X
  cv::Sobel( blurred_gray, grad_x, CV_32F, 1, 0, ker_size, scale, delta );
  /// Gradient Y
  cv::Sobel( blurred_gray, grad_y, CV_32F, 0, 1, ker_size, scale, delta );
  cv::convertScaleAbs( grad_x, abs_grad_x );
  cv::convertScaleAbs( grad_y, abs_grad_y );
  cv::addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, source );
}


#include "displayimage.moc"
