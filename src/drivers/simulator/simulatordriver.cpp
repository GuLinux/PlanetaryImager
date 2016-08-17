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

#include "simulatordriver.h"
#include "drivers/imagerthread.h"
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
#include "fps_counter.h"
#include "utils.h"
#include "Qt/functional.h"
#include <atomic>

using namespace GuLinux;
using namespace std;
using namespace std::chrono_literals;

class SimulatorCamera : public Driver::Camera {
public:
  virtual ImagerPtr imager ( const ImageHandlerPtr& imageHandler) const;
  virtual QString name() const { return "Simulator Camera"; }
};


class SimulatorImager : public Imager {
  Q_OBJECT
public:
    SimulatorImager(const ImageHandlerPtr &handler);
    virtual ~SimulatorImager();
    virtual Chip chip() const;
    virtual QString name() const;
    virtual void setSetting(const Setting& setting);
    virtual Settings settings() const;
    virtual void startLive();
    virtual void stopLive();
    int rand(int a, int b);
    QMap<QString, Imager::Setting> _settings;
    QMutex settingsMutex;
    virtual bool supportsROI() { return false; }
    
    class Worker : public ImagerThread::Worker {
    public:
      Worker(SimulatorImager *imager);
      bool shoot(const ImageHandlerPtr& imageHandler);
      virtual void start();
      virtual void stop();
    private:
      cv::Mat image;
      SimulatorImager *imager;
    };
    friend class Worker;
public slots:
    virtual void setROI(const QRect &) {}
    virtual void clearROI() {}
private:
  ImageHandlerPtr imageHandler;
  ImagerThread::ptr imager_thread;
};

ImagerPtr SimulatorCamera::imager ( const ImageHandlerPtr& imageHandler ) const
{
  return make_shared<SimulatorImager>(imageHandler);
}

SimulatorImager::~SimulatorImager()
{
  LOG_F_SCOPE
  dumpObjectInfo();
  stopLive();
  emit disconnected();
}



Driver::Cameras SimulatorDriver::cameras() const
{
  static shared_ptr<SimulatorCamera> simulatorCamera = make_shared<SimulatorCamera>();
  return {simulatorCamera};
}

SimulatorImager::SimulatorImager(const ImageHandlerPtr& handler) : imageHandler{handler}, _settings{
    {"exposure", {1, "exposure", 0, 100, 1, 50}},
    {"movement", {3, "movement", 0, 5, 1, 1}},
    {"seeing",   {4, "seeing", 0, 5, 1, 1}},
    {"delay",    {5, "delay", 0, 100, 1, 1}},
    {"bin",	 {6, "bin", 0, 3, 1, 1, 1, Setting::Combo, { {"1x1", 1}, {"2x2", 2}, {"3x3", 3}, {"4x4", 4} } }}, 
    {"temperature", {7, "temperature", 0, 300, 0.1, 30, 0} },
  }
{
  _settings["temperature"].decimals = 1;
  _settings["temperature"].readonly = true;
  _settings["exposure"].is_duration = true;
  _settings["exposure"].duration_unit = 1ms;
  _settings["seeing"].supports_auto = true;
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
  qDebug() << "Received setting: \n" << setting << "\n saved setting: \n" << _settings[setting.name];
  _settings[setting.name] = setting;
  emit changed(setting);
}

Imager::Settings SimulatorImager::settings() const
{
  auto valid_settings = _settings.values();
  valid_settings.erase(remove_if(valid_settings.begin(), valid_settings.end(), [](const Setting &s){ return s.name.isEmpty(); }), valid_settings.end());
  return valid_settings;
}


int SimulatorImager::rand(int a, int b)
{
   return qrand() % ((b + 1) - a) + a;
}


void SimulatorImager::Worker::start()
{
}

void SimulatorImager::Worker::stop()
{
}

SimulatorImager::Worker::Worker(SimulatorImager* imager) : imager{imager}
{
    QFile file(":/simulator/jupiter.png");
    file.open(QIODevice::ReadOnly);
    QByteArray file_data = file.readAll();
    image = cv::imdecode(cv::InputArray{file_data.data(), file_data.size()}, CV_LOAD_IMAGE_COLOR);
}


bool SimulatorImager::Worker::shoot(const ImageHandlerPtr &imageHandler)
{
  auto rand = [](int a, int b) { return qrand() % ((b + 1) - a) + a; };
  cv::Mat cropped, blurred, result;
  int h = image.rows;
  int w = image.cols;
  Setting exposure, gamma, delay, seeing, movement;
  {
      QMutexLocker lock_settings(&imager->settingsMutex);
      exposure = imager->_settings["exposure"];
      gamma = imager->_settings["gamma"];
      seeing = imager->_settings["seeing"];
      movement = imager->_settings["movement"];
      delay = imager->_settings["delay"];
  }
  int crop_factor = movement.value;
  int pix_w = rand(0, crop_factor);
  int pix_h = rand(0, crop_factor);

  cv::Rect crop_rect(0, 0, w, h);
  crop_rect -= cv::Size{crop_factor, crop_factor};
  crop_rect += cv::Point{pix_w, pix_h};
  cropped = image(crop_rect);
  if(rand(0, seeing.max) > (seeing.value_auto ? 3 : seeing.value) ) {
      auto ker_size = rand(1, 7);
      cv::blur(cropped, blurred, {ker_size, ker_size});
  } else {
      cropped.copyTo(blurred);
  }
      int exposure_offset = exposure.value * 2 - 100;
      result = blurred + cv::Scalar{exposure_offset, exposure_offset, exposure_offset};
  int depth = 8;
  if(result.depth() > CV_8S)
      depth = 16;
  if(result.depth() > CV_16S)
      depth = 32;
  auto scale = imager->_settings["bin"].value;
  if(scale > 1)
    cv::resize(result, result, {result.cols/scale, result.rows/scale});
  imageHandler->handle(result);
  QThread::msleep(delay.value);
  return true;
}



void SimulatorImager::startLive()
{
  LOG_F_SCOPE
  auto worker = make_shared<Worker>(this);
  imager_thread = make_shared<ImagerThread>(worker, this, imageHandler);
  imager_thread->start();
}

void SimulatorImager::stopLive()
{
  LOG_F_SCOPE
  imager_thread.reset();
}

SimulatorDriver::SimulatorDriver()
{
  Q_INIT_RESOURCE(simulator);
}



#include "simulatordriver.moc"
