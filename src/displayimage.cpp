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
#include "configuration.h"
#include <fps_counter.h>
#include <QThread>
#include <QElapsedTimer>
#include <QRect>
#include <QImage>
#include <boost/lockfree/spsc_queue.hpp>
#include <QDebug>
#include "Qt/strings.h"
#include "utils.h"
#include <opencv2/opencv.hpp>
#include "opencv_utils.h"
#include "c++/stlutils.h"
#include <atomic>
using namespace std;

DPTR_IMPL(DisplayImage) {
  Configuration &configuration;
  unique_ptr<fps_counter> capture_fps;
  DisplayImage *q;
  
  atomic_bool running;
  int milliseconds_limit = 0;
  QElapsedTimer elapsed;
  QRect imageRect;
  boost::lockfree::spsc_queue<cv::Mat, boost::lockfree::capacity<5>> queue;
  bool detectEdges = false;
  QVector<QRgb> grayScale;
  void canny(cv::Mat& source, int lowThreshold = 1, int ratio = 3, int kernel_size = 3, int blurSize = 3);
  void sobel( cv::Mat& source, int blur_size = 3, int ker_size = 3, int scale = 1, int delta = 0 );
};




DisplayImage::~DisplayImage()
{

}

DisplayImage::DisplayImage(Configuration& configuration, QObject* parent)
  : QObject(parent), dptr(configuration, make_unique<fps_counter>([=](double fps){ emit displayFPS(fps);}), this)
{
  d->running = true;
  setRecording(false);
  for(int i=0; i<0xff; i++)
    d->grayScale.push_back(qRgb(i, i, i));
}
void DisplayImage::setRecording(bool recording)
{
  int fps = recording ? d->configuration.maxPreviewFPSOnSaving() : 0;
  d->milliseconds_limit = (fps == 0 ? 1000./40. : 1000/fps);
  d->elapsed.restart();
}

void DisplayImage::handle( const cv::Mat& imageData )
{
  if( (d->milliseconds_limit > 0 && d->elapsed.elapsed() < d->milliseconds_limit) || !imageData.data || ! d->queue.push(imageData) ) {
    return;
  }
  d->elapsed.restart();
}

void DisplayImage::create_qimages()
{
  cv::Mat imageData;
  while(d->running) {
    if(!d->queue.pop(imageData)) {
      QThread::msleep(1);
      continue;
    }
    if(imageData.depth() != CV_8U && imageData.depth() != CV_8S) {
      imageData.convertTo(imageData, CV_8U, 0.00390625); // TODO: handle color images
    }
    ++*d->capture_fps;
//     cv::Mat origin{imageData->height(), imageData->width(), imageData->channels() == 1 ? CV_8UC1 : CV_8UC3, imageData->data()};
    auto cv_image = new cv::Mat;
    cv::cvtColor(imageData, *cv_image, imageData.channels() == 1 ? CV_GRAY2RGB : CV_BGR2RGB);
    if(d->detectEdges) {
      if(d->configuration.edgeAlgorithm() == Configuration::Sobel) {
	benchmark_scope(sobel)
	d->sobel(*cv_image, d->configuration.sobelBlurSize(), d->configuration.sobelKernel(), d->configuration.sobelScale(), d->configuration.sobelDelta());
      } else if(d->configuration.edgeAlgorithm() == Configuration::Canny) {
	benchmark_scope(canny)
	d->canny(*cv_image, d->configuration.cannyLowThreshold(), d->configuration.cannyThresholdRatio(), d->configuration.cannyKernelSize(), d->configuration.cannyBlurSize());
      }
    }
    QImage image{cv_image->data, cv_image->cols, cv_image->rows, cv_image->step, cv_image->channels() == 1 ? QImage::Format_Indexed8: QImage::Format_RGB888, 
      [](void *data){ delete reinterpret_cast<cv::Mat*>(data); }, cv_image};
    if(cv_image->channels() == 1) {
      image.setColorTable(d->grayScale);
    }
    d->imageRect = image.rect();
    emit gotImage(image);
  }
  QThread::currentThread()->quit();
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