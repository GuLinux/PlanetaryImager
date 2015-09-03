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
#include "utils.h"

#include <QtConcurrent/QtConcurrent>
#include <functional>
#include <cstring>
#include "fps_counter.h"
#include "configuration.h"
#include <boost/lockfree/spsc_queue.hpp>
#include "ser_header.h"
#include "opencv_utils.h"
#include <Qt/strings.h>


using namespace std;
using namespace std::placeholders;

class FileWriter : public ImageHandler {
public:
  virtual void handle(const cv::Mat& imageData) = 0;
  virtual QString filename() const = 0;
};
typedef shared_ptr<FileWriter> FileWriterPtr;
typedef function<FileWriterPtr(const QString &deviceName, Configuration &configuration)> FileWriterFactory;



class SER_Writer : public FileWriter {
public:
  SER_Writer(const QString &deviceName, Configuration &configuration);
  ~SER_Writer();
  virtual void handle(const cv::Mat& imageData);
  virtual QString filename() const;
  vector<SER_Timestamp> timestamps;
private:
  QFile file;
  SER_Header *header;
  uint32_t frames = 0;
};

class cvVideoWriter : public FileWriter {
public:
  cvVideoWriter(const QString &deviceName, Configuration &configuration);
  ~cvVideoWriter();
  virtual void handle(const cv::Mat& imageData);
  virtual QString filename() const { return _filename; }
private:
  QString _filename;
  Configuration &configuration;
  shared_ptr<cv::VideoWriter> videoWriter;
};



SER_Writer::SER_Writer(const QString &deviceName, Configuration &configuration) : file(configuration.savefile())
{
  qDebug() << "Using buffered output: " << configuration.bufferedOutput();
  if(configuration.bufferedOutput())
    file.open(QIODevice::ReadWrite);
  else
    file.open(QIODevice::ReadWrite | QIODevice::Unbuffered);
  SER_Header empty_header;
  empty_header.datetime = QDateTime({1, 1, 1}, {0,0,0}).msecsTo(QDateTime::currentDateTime()) * 10000;
  empty_header.datetime_utc = QDateTime({1, 1, 1}, {0,0,0}, Qt::UTC).msecsTo(QDateTime::currentDateTimeUtc()) * 10000;
  ::strcpy(empty_header.camera, deviceName.left(40).toLatin1());
  ::strcpy(empty_header.observer, configuration.observer().left(40).toLatin1());
  ::strcpy(empty_header.telescope, configuration.telescope().left(40).toLatin1());
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
  for(auto timestamp: timestamps) {
    file.write(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
  }
  file.close();
  qDebug() << "file correctly closed.";
}

QString SER_Writer::filename() const
{
  return file.fileName();
}

void SER_Writer::handle(const cv::Mat& imageData)
{
  if(! header->imageWidth) {
    header->colorId = imageData.channels() == 1 ? SER_Header::MONO : SER_Header::RGB;
    header->pixelDepth = 8; // TODO imageData->bpp();
    header->imageWidth = imageData.cols;
    header->imageHeight = imageData.rows;
  }
  timestamps.push_back(QDateTime({1, 1, 1}, {0,0,0}, Qt::UTC).msecsTo(QDateTime::currentDateTimeUtc()) * 10000);
  file.write(reinterpret_cast<const char*>(imageData.data), imageData.total() * imageData.elemSize());
  ++frames;
}


cvVideoWriter::cvVideoWriter ( const QString& deviceName, Configuration& configuration ) : _filename(configuration.savefile()), configuration{configuration}
{
  qDebug() <<__PRETTY_FUNCTION__ << "filename: " << filename();
}

void cvVideoWriter::handle ( const cv::Mat& imageData )
{
  try {
    if(!videoWriter)
      videoWriter = make_shared<cv::VideoWriter>(_filename.toStdString(), CV_FOURCC('X', '2', '6', '4'), 25, cv::Size{imageData.cols, imageData.rows});
    if(!videoWriter || ! videoWriter->isOpened()) {
      qWarning() << "unable to open video file" << _filename;
      return;
    }
    *videoWriter << imageData;
  } catch(cv::Exception &e) {
    qWarning() << "error on handle:" << e.msg << e.code << e.file << e.line << e.func;
  }
}

cvVideoWriter::~cvVideoWriter()
{
  videoWriter->release();
}



class WriterThreadWorker;
class SaveImages::Private {
public:
    Private(Configuration &configuration, SaveImages *q);
    Configuration &configuration;
    QThread recordingThread;
    FileWriterFactory writerFactory();
    WriterThreadWorker *worker = nullptr;
    bool is_recording = false;
private:
    SaveImages *q;
};

SaveImages::Private::Private(Configuration &configuration, SaveImages* q) : configuration{configuration}, q{q}
{
}

SaveImages::SaveImages(Configuration& configuration, QObject* parent) : QObject(parent), dptr(configuration, this)
{
}


SaveImages::~SaveImages()
{
}


FileWriterFactory SaveImages::Private::writerFactory()
{
  if(configuration.savefile().isEmpty()) {
    return {};
  }
  static map<Configuration::SaveFormat, FileWriterFactory> factories {
    {Configuration::SER, [](const QString &deviceName, Configuration &configuration){ return make_shared<SER_Writer>(deviceName, configuration); }},
    {Configuration::Video, [](const QString &deviceName, Configuration &configuration){ return make_shared<cvVideoWriter>(deviceName, configuration); }},
  };
  
  return factories[configuration.saveFormat()];
}

class WriterThreadWorker : public QObject {
  Q_OBJECT
public:
  explicit WriterThreadWorker ( const function< FileWriterPtr() >& fileWriterFactory, uint64_t max_frames, long long int max_memory, bool& is_recording, SaveImages *saveImages, QObject* parent = 0 );
  virtual ~WriterThreadWorker();
public slots:
  virtual void handle(const cv::Mat& imageData);
  void run();
private:
  function<FileWriterPtr()> fileWriterFactory;
  shared_ptr<boost::lockfree::spsc_queue<cv::Mat>> framesQueue;
  uint64_t max_frames;
  long long max_memory;
  bool &is_recording;
  uint64_t dropped_frames = 0;
  SaveImages *saveImages;
};

WriterThreadWorker::WriterThreadWorker ( const function<FileWriterPtr()>& fileWriterFactory, uint64_t max_frames, long long int max_memory, bool& is_recording, SaveImages* saveImages, QObject* parent )
  : QObject(parent), fileWriterFactory(fileWriterFactory), max_frames(max_frames), max_memory{max_memory}, is_recording{is_recording}, saveImages{saveImages}
{
}

WriterThreadWorker::~WriterThreadWorker()
{
}

#define MB(arg) (arg* 1024 * 1024)

void WriterThreadWorker::handle(const cv::Mat& imageData)
{
  auto imageDataSize = imageData.total()* imageData.elemSize();
  if(!framesQueue) {
    framesQueue = make_shared<boost::lockfree::spsc_queue<cv::Mat>>(max_memory/imageDataSize);
    qDebug() << "allocated framesqueue with " << max_memory << " bytes capacity (" << max_memory/imageDataSize << " frames)";
  }
  
  if(!framesQueue->push(imageData)) {
    qWarning() << "Frames queue too high, dropping frame";
    emit saveImages->droppedFrames(++dropped_frames);
  }
}


void WriterThreadWorker::run()
{
  {
    auto fileWriter = fileWriterFactory();
    fps_counter savefps{[=](double fps){ emit saveImages->saveFPS(fps);}, fps_counter::Elapsed};
    fps_counter meanfps{[=](double fps){ emit saveImages->meanFPS(fps);}, fps_counter::Elapsed, 1000, true};
    uint64_t frames = 0;
    emit saveImages->recording(fileWriter->filename());
    while(is_recording && frames < max_frames) {
      cv::Mat frame;
      if(framesQueue && framesQueue->pop(frame)) {
	  fileWriter->handle(frame);
	++savefps;
	++meanfps;
	emit saveImages->savedFrames(++frames);
      } else {
	QThread::msleep(1);
      }
    }
    is_recording = false;
  }
  qDebug() << "closing thread";
  emit saveImages->finished();
  QThread::currentThread()->quit();
  qDebug() << "finished worker";
}


void SaveImages::handle(const cv::Mat& imageData)
{
  if(!d->is_recording)
    return;
  QtConcurrent::run(bind(&WriterThreadWorker::handle, d->worker, imageData));
}

void SaveImages::startRecording(const QString &deviceName)
{
  auto writerFactory = d->writerFactory();
  if(writerFactory) {
    d->worker = new WriterThreadWorker(bind(writerFactory, 
					    deviceName, 
					    std::ref<Configuration>(d->configuration)),
				       d->configuration.recordingFramesLimit() == 0 ? std::numeric_limits<long long>().max() : d->configuration.recordingFramesLimit(), 
				       d->configuration.maxMemoryUsage(), 
				       d->is_recording, this);

    connect(&d->recordingThread, &QThread::finished, bind(&SaveImages::finished, this));
    d->worker->moveToThread(&d->recordingThread);
    connect(&d->recordingThread, &QThread::started, d->worker, &WriterThreadWorker::run);
    connect(&d->recordingThread, &QThread::finished, d->worker, &WriterThreadWorker::deleteLater);
    d->is_recording = true;
    d->recordingThread.start();
  }
}



void SaveImages::endRecording()
{
  d->is_recording = false;
}


#include "saveimages.moc"
