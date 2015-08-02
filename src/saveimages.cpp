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

#include "saveimages.h"
#include <QFile>
#include <QThread>
#include <QDebug>
#include <functional>
#include <QQueue>
#include "utils.h"

#include <QMutex>
#include <QMutexLocker>
#include <QtConcurrent/QtConcurrent>

using namespace std;

class FileWriter : public ImageHandler {
public:
  virtual void handle(const ImageDataPtr& imageData) = 0;
};
typedef shared_ptr<FileWriter> FileWriterPtr;
typedef function<FileWriterPtr(const QString &filename)> FileWriterFactory;

class WriterThreadWorker : public QObject {
  Q_OBJECT
public:
  explicit WriterThreadWorker ( const FileWriterPtr &fileWriter, QObject* parent = 0 ) : QObject(parent), fileWriter(fileWriter) {}
public slots:
  virtual void handle(const ImageDataPtr& imageData) { frames.enqueue(imageData); }
  void finish() {stop = true; }
  void run();
signals:
  void saveFPS(double fps);
  void savedFrames(uint64_t frames);
private:
  FileWriterPtr fileWriter;
  QQueue<ImageDataPtr> frames;
  bool stop = false;
};

struct __attribute__ ((__packed__)) SER_Header {
    char fileId[14] = {'L', 'U', 'C', 'A', 'M', '-', 'R','E','C','O','R','D','E','R'};
    int32_t luId = 0;
    enum ColorId {
        MONO = 0,
        BAYER_RGGB = 8,
        BAYER_GRBG = 9,
        BAYER_GBRG = 10,
        BAYER_BGGR = 11,
        BAYER_CYYM = 16,
        BAYER_YCMY = 17,
        BAYER_YMCY = 18,
        BAYER_MYYC = 19,
        RGB = 100,
        BGR = 101,
    };
    int32_t colorId = MONO;
    enum Endian { BigEndian = 0, LittleEndian = 1 };
    int32_t endian = BigEndian;
    uint32_t imageWidth = 0;
    uint32_t imageHeight = 0;
    uint32_t pixelDepth = 0;
    uint32_t frames = 0;
    char observer[40] = {};
    char camera[40] = {};
    char telescope[40] = {};
    uint8_t datetime[8] = {};
    uint8_t datetime_utc[8] = {};
};

class SER_Writer : public FileWriter {
public:
  SER_Writer(const QString &filename);
  ~SER_Writer();
  virtual void handle(const ImageDataPtr& imageData);
private:
  SER_Header *header;
  QFile file;
  bool wrote_image_data = false;
};


class SaveImages::Private {
public:
    Private(SaveImages *q);
    QThread recordingThread;
    QString filename;
    Format format = SER;
    FileWriterPtr createWriter();
    WriterThreadWorker *worker = nullptr;
private:
    SaveImages *q;
};

SaveImages::Private::Private(SaveImages* q) : q {q}
{
}

SaveImages::SaveImages(QObject* parent) : QObject(parent), dpointer(this)
{
}


SaveImages::~SaveImages()
{
}


void SaveImages::setOutput(const QString& filename, Format format)
{
  d->filename = filename;
  d->format = format;
}

FileWriterPtr SaveImages::Private::createWriter()
{
  if(filename.isEmpty()) {
    return {};
  }
  static map<Format, FileWriterFactory> factories {
    {SER, [](const QString &filename){ return make_shared<SER_Writer>(filename); }},
  };
  
  return factories[format](filename);
}

SER_Writer::SER_Writer(const QString& filename) : file("%1.ser"_q % filename)
{
  file.open(QIODevice::ReadWrite);

  SER_Header header;
  file.write(reinterpret_cast<char*>(&header), sizeof(header));
  file.flush();
  this->header = reinterpret_cast<SER_Header*>(file.map(0, sizeof(header)));
  if(!this->header) {
    qDebug() << file.errorString();
  }
}

SER_Writer::~SER_Writer()
{
  file.close();
}


void SER_Writer::handle(const ImageDataPtr& imageData)
{
  if(! wrote_image_data) {
    header->colorId = imageData->channels() == 1 ? SER_Header::MONO : SER_Header::RGB;
    header->pixelDepth = imageData->bpp();
    header->imageWidth = imageData->width();
    header->imageHeight = imageData->height();
    wrote_image_data = true;
  }
  header->frames++;
  file.write(reinterpret_cast<const char*>(imageData->data()), imageData->size());
}


void WriterThreadWorker::run()
{
  fps savefps{[=](double fps){ emit saveFPS(fps);}};
  uint64_t frames = 0;
  while(!stop) {
    if(this->frames.size()>0) {
      fileWriter->handle(this->frames.dequeue());
        emit savedFrames(++frames);
    }
  }
}


void SaveImages::handle(const ImageDataPtr& imageData)
{
  if(!d->worker)
    return;
  d->worker->handle(imageData);
}

void SaveImages::startRecording()
{
  auto writer = d->createWriter();
  if(writer) {
    d->worker = new WriterThreadWorker(writer);
    connect(d->worker, SIGNAL(savedFrames(uint64_t)), this, SIGNAL(savedFrames(uint64_t)));
    connect(d->worker, SIGNAL(saveFPS(double)), this, SIGNAL(saveFPS(double)));
    d->worker->moveToThread(&d->recordingThread);
    connect(&d->recordingThread, SIGNAL(started()), d->worker, SLOT(run()));
    connect(&d->recordingThread, SIGNAL(finished()), d->worker, SLOT(deleteLater()));
    d->recordingThread.start();
  }
}


void SaveImages::endRecording()
{
  d->worker->finish();
  d->recordingThread.quit();
  QtConcurrent::run([=] {
    d->recordingThread.wait();
    d->recordingThread.terminate();
    d->worker = nullptr;
  });
}


#include "saveimages.moc"
