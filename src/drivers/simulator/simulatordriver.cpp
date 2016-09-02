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
#include <chrono>
#include <unordered_map>


using namespace GuLinux;
using namespace std;
using namespace std::chrono_literals;

class SimulatorCamera : public Driver::Camera {
public:
  virtual Imager * imager ( const ImageHandlerPtr& imageHandler) const;
  virtual QString name() const { return "Simulator Camera"; }
};


class SimulatorImager : public Imager {
  Q_OBJECT
public:
    SimulatorImager(const ImageHandlerPtr &handler);
    virtual ~SimulatorImager();
    virtual Chip chip() const;
    virtual QString name() const;
    virtual void setControl(const Control& setting);
    virtual Controls controls() const;
    virtual void startLive();
    virtual void stopLive();
    static int rand(int a, int b);
    QMap<QString, Imager::Control> _settings;
    QMutex settingsMutex;
    virtual bool supportsROI() { return true; }
    
    class Worker : public ImagerThread::Worker {
    public:
      Worker(SimulatorImager *imager);
      bool shoot(const ImageHandlerPtr& imageHandler);
      virtual void start();
      virtual void stop();
      void setROI(const QRect &roi);
      enum ImageType{ BGR = 0, Mono = 10, Bayer = 20};
    private:
      QHash<int, cv::Mat> images;
      SimulatorImager *imager;
      QRect roi;
    };
    friend class Worker;
    QTimer refresh_temperature;
public slots:
    virtual void setROI(const QRect &);
    virtual void clearROI();
private:
  ImageHandlerPtr imageHandler;
  ImagerThread::ptr imager_thread;
  shared_ptr<Worker> worker;
};

Imager * SimulatorCamera::imager ( const ImageHandlerPtr& imageHandler ) const
{
  return new SimulatorImager(imageHandler);
}

SimulatorImager::~SimulatorImager()
{
}



Driver::Cameras SimulatorDriver::cameras() const
{
  static shared_ptr<SimulatorCamera> simulatorCamera = make_shared<SimulatorCamera>();
  return {simulatorCamera};
}

SimulatorImager::SimulatorImager(const ImageHandlerPtr& handler) : imageHandler{handler}, _settings{
    {"exposure",    {1, "exposure", 0.1, 1000, 0.1, 19.5}},
    {"movement", {2, "movement", 0, 5, 1, 1}},
    {"seeing",   {3, "seeing", 0, 5, 1, 1}},
    {"bin",	 {4, "bin", 0, 3, 1, 1, 1, Control::Combo, { {"1x1", 1}, {"2x2", 2}, {"3x3", 3}, {"4x4", 4} } }}, 
    {"format",	 {5, "format", 0, 100, 1, Worker::Mono, Worker::Mono, Control::Combo, { {"Mono", Worker::Mono}, {"BGR", Worker::BGR},  {"Bayer", Worker::Bayer}} }}, 
  }
{
  qDebug() << "Creating simulator imager: current owning thread: " << thread() << ", qApp thread: " << qApp->thread();
  _settings["exposure"].is_duration = true;
  _settings["exposure"].duration_unit = 1ms;
  _settings["seeing"].supports_auto = true;
  
  connect(&refresh_temperature, &QTimer::timeout, this, [this]{
    if(!imager_thread)
      return; // TODO: ugly fix
    imager_thread->push_job([&]{
      double celsius = SimulatorImager::rand(200, 500) / 10.;
      emit temperature(celsius);
    });
  });
  refresh_temperature.start(2000);
}


Imager::Chip SimulatorImager::chip() const
{
  return {};
}

QString SimulatorImager::name() const
{
  return "Simulator Imager";
}

void SimulatorImager::setControl(const Imager::Control& setting)
{
  QMutexLocker lock_settings(&settingsMutex);
  qDebug() << "Received setting: \n" << setting << "\n saved setting: \n" << _settings[setting.name];
  _settings[setting.name] = setting;
  emit changed(setting);
}

Imager::Controls SimulatorImager::controls() const
{
  return _settings.values();
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
    QFile file(":/simulator/jupiter_hubble.jpg");
    file.open(QIODevice::ReadOnly);
    QByteArray file_data = file.readAll();
    images[BGR] = cv::imdecode(cv::InputArray{file_data.data(), file_data.size()}, CV_LOAD_IMAGE_COLOR);
    cv::cvtColor(images[BGR], images[Mono], CV_BGR2GRAY);
    images[Bayer] = cv::Mat(images[BGR].rows, images[BGR].cols, CV_8UC1);
    
    QHash<QPair<int,int>, int> bayer_pattern_channels {
      // B=0, G=1, R=2
      { {0, 0}, 2 }, { {0, 1}, 1 }, { {1, 0}, 1 }, { {1, 1}, 0}
    };
    for(int row = 0; row < images[BGR].rows; row++) {
      for(int column = 0; column < images[BGR].cols; column++) {
        int channel = bayer_pattern_channels[{row%2, column%2}];
        images[Bayer].at<uint8_t>(row, column) = images[BGR].at<cv::Vec3b>(row, column).val[channel];
      }
    }
    for(int bin = 1; bin < 5; bin++) {
      double ratio = 4. / bin;
      cv::resize(images[BGR], images[BGR + bin], {}, ratio, ratio);
      cv::resize(images[Mono], images[Mono + bin], {}, ratio, ratio);
    }
}


bool SimulatorImager::Worker::shoot(const ImageHandlerPtr &imageHandler)
{
  static map<Worker::ImageType, Frame::ColorFormat> formats {
    {Worker::Mono, Frame::Mono},
    {Worker::BGR, Frame::BGR},
    {Worker::Bayer, Frame::Bayer_RGGB},
  };
  auto rand = [](int a, int b) { return qrand() % ((b + 1) - a) + a; };
  cv::Mat cropped, blurred, result;
  Control exposure, seeing, movement, bin, format;
  {
      QMutexLocker lock_settings(&imager->settingsMutex);
      exposure = imager->_settings["exposure"];
      bin = imager->_settings["bin"];
      format = imager->_settings["format"];
      seeing = imager->_settings["seeing"];
      movement = imager->_settings["movement"];
  }
  bool is_bayer = static_cast<ImageType>(format.value) == Bayer;
  const cv::Mat &image = is_bayer ? images[Bayer] : images[format.value + bin.value];
  int h = image.rows;
  int w = image.cols;
  int crop_factor = movement.value;
  int pix_w = rand(0, crop_factor);
  int pix_h = rand(0, crop_factor);

  cv::Rect crop_rect(0, 0, w, h);
  crop_rect -= cv::Size{crop_factor, crop_factor};
  crop_rect += cv::Point{pix_w, pix_h};
  cropped = is_bayer ? image : image(crop_rect);
  if(roi.isValid() && ! is_bayer) {
    cropped = cropped(cv::Rect{roi.x(), roi.y(), roi.width(), roi.height()});
  }
  if(! is_bayer && (rand(0, seeing.max) > (seeing.value_auto ? 3 : seeing.value) ) ) {
      auto ker_size = rand(1, 7);
      cv::blur(cropped, blurred, {ker_size, ker_size});
  } else {
      cropped.copyTo(blurred);
  }
  double exposure_percent = (exposure.value - exposure.min) / (exposure.max - exposure.min) * 100;
  double exposure_offset = log(exposure_percent)*150 - 100;
  static auto started = chrono::steady_clock::now();
  auto now = chrono::steady_clock::now();
  if( (now-started) >= 1s) {
    qDebug() << "Exposure percent: " << exposure_percent << ", exposure offset: " << exposure_offset;
    started = now;
  }
  result = blurred + cv::Scalar{exposure_offset, exposure_offset, exposure_offset};
  int depth = 8;
  if(result.depth() > CV_8S)
      depth = 16;
  if(result.depth() > CV_16S)
      depth = 32;
  imageHandler->handle(Frame::create(result, formats[static_cast<Worker::ImageType>(format.value)]));
  QThread::usleep(exposure.value * 1000);
  return true;
}

void SimulatorImager::clearROI()
{
  worker->setROI({});
}

void SimulatorImager::setROI(const QRect &roi)
{
  worker->setROI(roi);
}

void SimulatorImager::Worker::setROI(const QRect& roi)
{
  this->roi = roi;
}



void SimulatorImager::startLive()
{
  LOG_F_SCOPE
  qDebug() << "Creating simulator imager: current owning thread: " << thread() << ", qApp thread: " << qApp->thread() << ", timer thread: " << refresh_temperature.thread() << ", current thread: " << QThread::currentThread();
  worker = make_shared<Worker>(this);
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
