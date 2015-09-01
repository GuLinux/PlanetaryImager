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
#include <Magick++.h>

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

SimulatorImager::SimulatorImager(const ImageHandlerPtr& handler) : imageHandler{handler}
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

}

Imager::Settings SimulatorImager::settings() const
{
  return {};
}
#include <QLabel>
void SimulatorImager::startLive()
{
  QFile file(":/simulator/jupiter.png");
  file.open(QIODevice::ReadOnly);
  QByteArray file_data = file.readAll();
  qDebug() << "read " << file_data.size() << "bytes: " << file_data.mid(0, 5);
    
  started = true;
  QtConcurrent::run([=]{
    Magick::Blob blob(file_data.data(), file_data.size());
    qDebug() << "blob created";
    auto image = make_shared<Magick::Image>(blob);
    qDebug() << "image created: " << image->isValid();
    int h = image->size().height();
    int w = image->size().width();
    qDebug() << "image valid: " << image->isValid() << ", " << w << "x" << h;
    while(started) {
      Magick::Blob writeBlob;
      image->write(&writeBlob, "GRAY", 8);
      auto imageData = ImageData::create(w, h, 8, 1, reinterpret_cast<const uint8_t*>(writeBlob.data()));
      imageHandler->handle(imageData);
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
  Magick::InitializeMagick(0);
  Q_INIT_RESOURCE(simulator);
}
