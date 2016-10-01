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

#include "serimager.h"
#include <QFileDialog>
#include "drivers/imagerthread.h"
#include <QFile>
#include "commons/ser_header.h"
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>
#include <QFileInfo>

using namespace std;

DPTR_IMPL(SERImager) {
};


class SERImagerWorker : public ImagerThread::Worker {
public:
  SERImagerWorker(const QString &file);
  ~SERImagerWorker();
  Frame::ptr shoot() override;
private:
  QFile file;
  SER_Header header;
  size_t frame_size;
  Frame::ColorFormat color_format;
  QSize resolution;
  int pixel_depth;
  int current_frame = 0;
  double fps = 30;
  vector<SER_Timestamp> timestamps;
};

SERImagerWorker::SERImagerWorker(const QString& file) : file{file}
{
  this->file.open(QIODevice::ReadOnly);
  this->file.read(reinterpret_cast<char*>(&header), sizeof(header));
  frame_size = header.frame_size();
  color_format = header.frame_color_format();
  resolution = {static_cast<int>(header.imageWidth), static_cast<int>(header.imageHeight)};
  pixel_depth = header.pixelDepth;
  size_t trailer_start = sizeof(SER_Header) + header.frames * frame_size;
  if(this->file.size() == trailer_start + sizeof(SER_Timestamp) * header.frames) {
    this->file.seek(trailer_start);
    timestamps.resize(header.frames);
    for(auto &timestamp: timestamps) {
      this->file.read(reinterpret_cast<char*>(&timestamp), sizeof(SER_Timestamp));
    }
    auto timestamps_diff = static_cast<double>(*timestamps.rbegin() - *timestamps.begin()) / 10000.;
    fps = 1000. /  (timestamps_diff / static_cast<double>(timestamps.size()) );
    qDebug() << "Have timestamps: duration in msecs=" << timestamps_diff << ", fps: " << fps;
  }
  qDebug() << "Opened SER file: " << header.frames << " frames";
}

Frame::ptr SERImagerWorker::shoot()
{
  QElapsedTimer elapsed;
  elapsed.start();
  auto frame = make_shared<Frame>(pixel_depth, color_format, resolution);
  file.seek(sizeof(header) + (frame_size * current_frame++) );
  file.read(reinterpret_cast<char*>(frame->data()), frame_size);
  if(current_frame >= header.frames)
    current_frame = 0;
  QThread::currentThread()->msleep( (1.0 / fps * 1000.) - elapsed.elapsed() );
  return frame;
}

SERImagerWorker::~SERImagerWorker()
{
}



SERImager::SERImager(const ImageHandler::ptr& handler) : Imager{handler}, dptr()
{
}

SERImager::~SERImager()
{
}

Imager::Properties SERImager::properties() const
{
  return {};
}

void SERImager::clearROI()
{
}

void SERImager::setROI(const QRect&)
{
}

Imager::Controls SERImager::controls() const
{
  return {};
}

QString SERImager::name() const
{
  return "SER Player";
}

void SERImager::setControl(const Imager::Control& setting)
{
}

void SERImager::startLive()
{
  QString ser_file = QFileDialog::getOpenFileName(nullptr, "Open SER file", qgetenv("HOME"), "SER Files (*.ser)");
  restart([=]{ return make_shared<SERImagerWorker>(ser_file); });
}



