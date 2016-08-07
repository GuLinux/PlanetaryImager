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
#include "fps_counter.h"
#include "utils.h"
#include "Qt/functional.h"

using namespace GuLinux;
using namespace std;

class SimulatorCamera : public Driver::Camera {
public:
  virtual ImagerPtr imager ( const ImageHandlerPtr& imageHandler) const;
  virtual QString name() const { return "Simulator Camera"; }
};

class SimulatorImager : public Imager {
public:
    SimulatorImager(const ImageHandlerPtr &handler);
    virtual ~SimulatorImager() { stopLive(); }
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
  QThread *imaging_thread = nullptr;
};

ImagerPtr SimulatorCamera::imager ( const ImageHandlerPtr& imageHandler ) const
{
  return make_shared<SimulatorImager>(imageHandler);
}


Driver::Cameras SimulatorDriver::cameras() const
{
  qDebug() << __PRETTY_FUNCTION__;
  static shared_ptr<SimulatorCamera> simulatorCamera = make_shared<SimulatorCamera>();
  return {simulatorCamera};
}

SimulatorImager::SimulatorImager(const ImageHandlerPtr& handler) : imageHandler{handler}, _settings{
    {"exposure", {1, "exposure", 0, 100, 1, 50}},
    {"movement", {3, "movement", 0, 5, 1, 1}},
    {"seeing",   {4, "seeing", 0, 5, 1, 1}},
    {"delay",    {5, "delay", 0, 100, 1, 1}},
    {"bin",	 {6, "bin", 0, 3, 1, 1, 1, Setting::Combo, { {"1x1", 1}, {"2x2", 2}, {"3x3", 3}, {"4x4", 4} } }}, 
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
  emit changed(setting);
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
  Thread::Run<void>{[=] {
        QFile file(":/simulator/jupiter.png");
        file.open(QIODevice::ReadOnly);
        QByteArray file_data = file.readAll();
        const cv::Mat image = cv::imdecode(cv::InputArray{file_data.data(), file_data.size()}, CV_LOAD_IMAGE_COLOR);
        fps_counter fps([=](double rate){ emit this->fps(rate);}, fps_counter::Mode::Elapsed);
        while(started) {
        cv::Mat cropped, blurred, result;
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
        cropped = image(crop_rect);
        if(rand(0, seeing.max) > seeing.value) {
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
	auto scale = _settings["bin"].value;
	if(scale > 1)
	  cv::resize(result, result, {result.cols/scale, result.rows/scale});
        ++fps;
        imageHandler->handle(result);
        QThread::msleep(delay.value);
        }
        qDebug() << "Testing image: capture finished";
    }, []{}, [&](Thread *t){ imaging_thread = t; }
  };
}

void SimulatorImager::stopLive()
{
  if(imaging_thread) {
    this->started = false;
    imaging_thread->wait();
    imaging_thread = nullptr;
  }
}

SimulatorDriver::SimulatorDriver()
{
  Q_INIT_RESOURCE(simulator);
}



