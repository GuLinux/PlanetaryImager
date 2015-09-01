/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <marco.gulino@bhuman.it>
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

#include "simulatordriver.h"
#include <QDebug>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QImage>
#include <QColor>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <opencv2/opencv.hpp>
#include "opencv_utils.h"

using namespace std;

class SimulatorCamera : public Driver::Camera {
public:
  virtual ImagerPtr imager ( const ImageHandlerPtr& imageHandler) const;
  virtual QString name() const { return "Simulator Camera"; }
};

class SimulatorImager : public Imager {
public:
    SimulatorImager(const ImageHandlerPtr &handler);
    virtual Chip chip() const;
    virtual QString name() const;
    virtual void setSetting(const Setting& setting);
    virtual Settings settings() const;
    virtual void startLive();
    virtual void stopLive();
    int rand(int a, int b);
    QMap<QString, Imager::Setting> _settings;
    QMutex settingsMutex;
private:
  ImageHandlerPtr imageHandler;
    bool started = false;
};

ImagerPtr SimulatorCamera::imager ( const ImageHandlerPtr& imageHandler ) const
{
  static ImagerPtr _imager = make_shared<SimulatorImager>(imageHandler);
  return _imager;
}


Driver::Cameras SimulatorDriver::cameras() const
{
  qDebug() << __PRETTY_FUNCTION__;
  static shared_ptr<SimulatorCamera> simulatorCamera = make_shared<SimulatorCamera>();
  return {simulatorCamera};
}

SimulatorImager::SimulatorImager(const ImageHandlerPtr& handler) : imageHandler{handler}, _settings{
    {"exposure", {1, "exposure", 0, 100, 1, 1}},
    {"gamma",    {2, "gamma", 0, 3, 0.1, 1}},
    {"movement",    {3, "movement", 0, 5, 1, 1}},
    {"seeing",    {4, "seeing", 0, 5, 1, 1}},
    {"delay",    {5, "delay", 0, 100, 1, 1}},
  }
{
}


Imager::Chip SimulatorImager::chip() const
{
  return {};
}

QString SimulatorImager::name() const
{
  return "Simulator Imager";
}

void SimulatorImager::setSetting(const Imager::Setting& setting)
{
  QMutexLocker lock_settings(&settingsMutex);
  _settings[setting.name] = setting;
}

Imager::Settings SimulatorImager::settings() const
{
  return _settings.values();
}


int SimulatorImager::rand(int a, int b)
{
   return qrand() % ((b + 1) - a) + a;
}

void SimulatorImager::startLive()
{
    
  started = true;
  QtConcurrent::run([=]{
    QFile file(":/simulator/jupiter.png");
    file.open(QIODevice::ReadOnly);
    QByteArray file_data = file.readAll();
    while(started) {
      cv::Mat image = cv::imdecode(cv::InputArray{file_data.data(), file_data.size()}, CV_LOAD_IMAGE_COLOR);
      int h = image.rows;
      int w = image.cols;
      Setting exposure, gamma, delay, seeing, movement;
      {
	QMutexLocker lock_settings(&settingsMutex);
        exposure = _settings["exposure"];
	gamma = _settings["gamma"];
        seeing = _settings["seeing"];
        movement = _settings["movement"];
        delay = _settings["delay"];
      }
      int crop_factor = movement.value;
      int pix_w = rand(0, crop_factor);
      int pix_h = rand(0, crop_factor);

      cv::Rect crop_rect(0, 0, w, h);
      crop_rect -= cv::Size{crop_factor, crop_factor};
      crop_rect += cv::Point{pix_w, pix_h};
      cv::Mat cropped = image(crop_rect);
      cv::Mat blur;
      if(rand(0, seeing.max) > seeing.value) {
        auto ker_size = rand(1, 7);
        cv::blur(cropped, blur, {ker_size, ker_size});
      } else {
        cropped.copyTo(blur);
      }
      cv::Mat result = cv::Mat::zeros(blur.size(), blur.type());
      for( int y = 0; y < blur.rows; y++ ) {
        for( int x = 0; x < blur.cols; x++ ) {
            for( int c = 0; c < 3; c++ ) {
              result.at<cv::Vec3b>(y,x)[c] = cv::saturate_cast<uchar>( gamma.value * ( blur.at<cv::Vec3b>(y,x)[c]) + exposure.value ) ;
            }
        }
      }
      int depth = 8;
      if(result.depth() > CV_8S)
        depth = 16;
      if(result.depth() > CV_16S)
        depth = 32;
      imageHandler->handle(result);
      QThread::msleep(delay.value);
    }
    qDebug() << "Testing image: capture finished";
  });
}

void SimulatorImager::stopLive()
{
  this->started = false;
}

SimulatorDriver::SimulatorDriver()
{
  Q_INIT_RESOURCE(simulator);
}
