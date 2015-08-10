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
#include <functional>
#include <cstring>
#include "fps_counter.h"
#include "configuration.h"

using namespace std;
using namespace std::placeholders;

class FileWriter : public ImageHandler {
public:
  virtual void handle(const ImageDataPtr& imageData) = 0;
  virtual QString filename() const = 0;
};
typedef shared_ptr<FileWriter> FileWriterPtr;
typedef function<FileWriterPtr(const QString &filename, bool buffered)> FileWriterFactory;


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
  SER_Writer(const QString &filename, bool buffered);
  ~SER_Writer();
  virtual void handle(const ImageDataPtr& imageData);
  virtual QString filename() const;
private:
  QFile file;
  SER_Header *header;
  uint32_t frames = 0;
};



SER_Writer::SER_Writer(const QString& filename, bool buffered) : file("%1.ser"_q % filename)
{
  print_thread_id
  qDebug() << "Using buffered output: " << buffered;
  if(buffered)
    file.open(QIODevice::ReadWrite);
  else
    file.open(QIODevice::ReadWrite | QIODevice::Unbuffered);
  SER_Header empty_header;
  file.write(reinterpret_cast<char*>(&empty_header), sizeof(empty_header));
  file.flush();
  header = reinterpret_cast<SER_Header*>(file.map(0, sizeof(SER_Header)));
  if(!header) {
    qDebug() << file.errorString();
  }
}

SER_Writer::~SER_Writer()
{
  qDebug() << "closing file..";
  header->frames = frames;
  file.close();
  qDebug() << "file correctly closed.";
}

QString SER_Writer::filename() const
{
  return file.fileName();
}


void SER_Writer::handle(const ImageDataPtr& imageData)
{
  if(! header->imageWidth) {
    header->colorId = imageData->channels() == 1 ? SER_Header::MONO : SER_Header::RGB;
    header->pixelDepth = imageData->bpp();
    header->imageWidth = imageData->width();
    header->imageHeight = imageData->height();
  }
  file.write(reinterpret_cast<const char*>(imageData->data()), imageData->size());
  ++frames;
  if((frames % 100) == 0) {
    header->frames = frames;
    file.flush();
  }
}


class WriterThreadWorker;
class SaveImages::Private {
public:
    Private(Configuration &configuration, SaveImages *q);
    Configuration &configuration;
    QThread recordingThread;
    QString filename;
    Format format = SER;
    FileWriterFactory writerFactory();
    WriterThreadWorker *worker = nullptr;
    uint64_t max_frames = std::numeric_limits<uint64_t>().max();
    bool is_recording = false;
private:
    SaveImages *q;
};

SaveImages::Private::Private(Configuration &configuration, SaveImages* q) : configuration{configuration}, q{q}
{
}

SaveImages::SaveImages(Configuration& configuration, QObject* parent) : QObject(parent), dpointer(configuration, this)
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

FileWriterFactory SaveImages::Private::writerFactory()
{
  if(filename.isEmpty()) {
    return {};
  }
  static map<Format, FileWriterFactory> factories {
    {SER, [](const QString &filename, bool buffered){ return make_shared<SER_Writer>(filename, buffered); }},
  };
  
  return factories[format];
}

class WriterThreadWorker : public QObject {
  Q_OBJECT
public:
  explicit WriterThreadWorker ( const function< FileWriterPtr() >& fileWriterFactory, uint64_t max_frames, long long int max_memory, bool& is_recording, QObject* parent = 0 );
  virtual ~WriterThreadWorker();
public slots:
  virtual void handle(const ImageDataPtr& imageData);
  void run();
signals:
  void saveFPS(double fps);
  void savedFrames(uint64_t frames);
  void started(const QString &filename);
  void finished();
private:
  function<FileWriterPtr()> fileWriterFactory;
  QQueue<ImageDataPtr> framesQueue;
  QMutex mutex;
  uint64_t max_frames;
  long long max_memory;
  bool &is_recording;
};

WriterThreadWorker::WriterThreadWorker ( const function<FileWriterPtr()>& fileWriterFactory, uint64_t max_frames, long long max_memory, bool &is_recording, QObject *parent )
  : QObject(parent), fileWriterFactory(fileWriterFactory), max_frames(max_frames), max_memory{max_memory}, is_recording{is_recording}
{
}

WriterThreadWorker::~WriterThreadWorker()
{
}

#define MB(arg) (arg* 1024 * 1024)

void WriterThreadWorker::handle(const ImageDataPtr& imageData)
{
  print_thread_id
  QMutexLocker lock(&mutex);
  auto framesQueueSize = framesQueue.size();
  if(framesQueueSize> 0 && framesQueueSize * imageData->size() > max_memory) {
    qWarning() << "Frames queue too high (" << framesQueueSize << ", " <<  static_cast<double>(framesQueueSize * imageData->size())/MB(1) << " MB), dropping frame";
    return;
  }
  framesQueue.enqueue(imageData); 
}


void WriterThreadWorker::run()
{
  print_thread_id
  {
    auto fileWriter = fileWriterFactory();
    fps_counter savefps{[=](double fps){ emit saveFPS(fps);}, fps_counter::Elapsed};
    uint64_t frames = 0;
    emit started(fileWriter->filename());
    while(is_recording && frames < max_frames) {
      if(framesQueue.size()>0) {
	{
	  QMutexLocker lock(&mutex);
	  fileWriter->handle(framesQueue.dequeue());
	}
	savefps.frame();
	emit savedFrames(++frames);
      } else {
	QThread::msleep(1);
      }
    }
    is_recording = false;
  }
  qDebug() << "closing thread";
  emit finished();
  QThread::currentThread()->quit();
  qDebug() << "finished worker";
}


void SaveImages::handle(const ImageDataPtr& imageData)
{
  print_thread_id
  if(!d->is_recording)
    return;
  QtConcurrent::run(bind(&WriterThreadWorker::handle, d->worker, imageData));
}

void SaveImages::startRecording()
{
  print_thread_id
  auto writerFactory = d->writerFactory();
  if(writerFactory) {
    d->worker = new WriterThreadWorker(bind(writerFactory, d->filename, d->configuration.bufferedOutput()), d->max_frames, d->configuration.maxMemoryUsage(), d->is_recording);
    connect(d->worker, &WriterThreadWorker::started, bind(&SaveImages::recording, this, _1));

    connect(&d->recordingThread, &QThread::finished, bind(&SaveImages::finished, this));
    connect(d->worker, &WriterThreadWorker::savedFrames, bind(&SaveImages::savedFrames, this, _1));
    connect(d->worker, &WriterThreadWorker::saveFPS, bind(&SaveImages::saveFPS, this, _1));
    d->worker->moveToThread(&d->recordingThread);
    connect(&d->recordingThread, &QThread::started, d->worker, &WriterThreadWorker::run);
    connect(&d->recordingThread, &QThread::finished, d->worker, &WriterThreadWorker::deleteLater);
    d->is_recording = true;
    d->recordingThread.start();
  }
}

void SaveImages::setFramesLimit ( uint64_t max_frames )
{
  d->max_frames = max_frames > 0 ? max_frames : std::numeric_limits<uint64_t>().max();
}


void SaveImages::endRecording()
{
  d->is_recording = false;
}


#include "saveimages.moc"
