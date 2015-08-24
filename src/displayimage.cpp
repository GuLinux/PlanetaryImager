/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

class DisplayImage::Private {
public:
  Private(Configuration &configuration, DisplayImage *q);
  Configuration &configuration;
  fps_counter capture_fps;
  int milliseconds_limit = 0;
  QElapsedTimer elapsed;
  QRect imageRect;
  bool running = true;
  boost::lockfree::spsc_queue<ImageDataPtr, boost::lockfree::capacity<5>> queue;
    bool detectEdges;
private:
  DisplayImage *q;
};

DisplayImage::Private::Private(Configuration& configuration, DisplayImage* q) : configuration{configuration}, capture_fps{[=](double fps){ emit q->displayFPS(fps);}}, q{q}
{

}


DisplayImage::~DisplayImage()
{

}

DisplayImage::DisplayImage(Configuration& configuration, QObject* parent)
  : QObject(parent), dptr(configuration, this)
{
  setRecording(false);
}
void DisplayImage::setRecording(bool recording)
{
  int fps = recording ? d->configuration.maxPreviewFPSOnSaving() : 0;
  d->milliseconds_limit = (fps == 0 ? 1000./40. : 1000/fps);
  d->elapsed.restart();
}

void DisplayImage::handle(const ImageDataPtr& imageData)
{
  if( (d->milliseconds_limit > 0 && d->elapsed.elapsed() < d->milliseconds_limit) || !imageData || ! d->queue.push(imageData) ) {
    return;
  }
  d->elapsed.restart();
}

#include <Magick++.h>
#include "utils.h"
void DisplayImage::create_qimages()
{
  ImageDataPtr imageData;
  while(d->running) {
    if(!d->queue.pop(imageData)) {
      QThread::msleep(1);
      continue;
    }
    int w = imageData->width();
    int h= imageData->height();
    QtConcurrent::run([=] {
    
      auto b1 = new benchmark("create blob");
      Magick::Blob blob(imageData->data(), imageData->size());
      auto data = new uint8_t[w*h*4]{0};
      delete b1;
      try {
	auto b2 = new benchmark("create image");
	Magick::Image image(blob, {w, h}, imageData->bpp(), imageData->channels() == 1 ? "GRAY" : "RGB");
	delete b2;
	if(d->detectEdges)
	  image.edge();
	auto b3 = new benchmark("write image");
	image.write(0, 0, w, h, "RGBA", Magick::StorageType::CharPixel, data);
	delete b3;
      } catch(std::exception &e) {
	qWarning() << e.what();
	delete [] data;
	return;
      }
      ++d->capture_fps;
      QImage qimage{data, w, h, QImage::Format_RGBX8888, [](void *data){ delete reinterpret_cast<uint8_t*>(data); }, data};
      d->imageRect = qimage.rect();
      emit gotImage(qimage);
    });
  }
  QThread::currentThread()->quit();
}
/*
void DisplayImage::create_qimages_qt55()
{

  ImageDataPtr imageData;
  while(d->running) {
    if(!d->queue.pop(imageData)) {
      QThread::msleep(1);
      continue;
    }
    ++d->capture_fps;
    ImageDataPtr *ptrCopy = new ImageDataPtr(imageData);
    QImage image{ptrCopy->get()->data(), ptrCopy->get()->width(), ptrCopy->get()->height(), QImage::Format_Grayscale8, [](void *data){ delete reinterpret_cast<ImageDataPtr*>(data); }, ptrCopy};
    d->imageRect = image.rect();
    emit gotImage(image);
  }
  QThread::currentThread()->quit();
}

*/
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



#include "displayimage.moc"