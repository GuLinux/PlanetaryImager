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

#include "local_saveimages.h"
#include <QFile>
#include <QThread>
#include <QDebug>
#include <functional>
#include "commons/utils.h"

#include <QtConcurrent/QtConcurrent>
#include <functional>
#include <cstring>
#include "commons/fps_counter.h"
#include "commons/configuration.h"
#include <boost/lockfree/spsc_queue.hpp>
#include "commons/opencv_utils.h"
#include <Qt/strings.h>
#include "output_writers/filewriter.h"
#include "recordinginformation.h"
#include <atomic>
#include "c++/stlutils.h"

using namespace std;
using namespace std::placeholders;
namespace {
class WriterThreadWorker;
};

DPTR_IMPL(LocalSaveImages) {
    Configuration::ptr configuration;
    WriterThreadWorker *worker;
    QThread *recordingThread;
    LocalSaveImages *q;
    FileWriter::Factory writerFactory();
};


namespace {
typedef boost::lockfree::spsc_queue<Frame::ptr> FramesQueue;
typedef function< FileWriter::Ptr() > CreateFileWriter;

struct RecordingParameters {
  CreateFileWriter fileWriterFactory;
  RecordingInformation::ptr recording_information;
  Configuration::RecordingLimit limit_type;
  int64_t max_frames;
  std::chrono::duration<double> max_seconds;
  bool write_txt_info;
  bool write_json_info;
  int64_t max_size;
  bool timelapse;
  qlonglong timelapse_msecs;
  Configuration::ptr configuration;
  RecordingInformation::Writer::ptr recording_information_writer(const FileWriter::Ptr &file_writer) const;
};

RecordingInformation::Writer::ptr RecordingParameters::recording_information_writer(const FileWriter::Ptr &file_writer) const {
  QList<RecordingInformation::Writer::ptr> writers;
  if(write_txt_info)
    writers.push_back(RecordingInformation::txt(file_writer->filename()));
  if(write_json_info)
    writers.push_back(RecordingInformation::json(file_writer->filename(), configuration));
  return RecordingInformation::composite(writers);
}

class Recording {
public:
  typedef shared_ptr<Recording> ptr;
    Recording(const RecordingParameters &parameters, LocalSaveImages *saveImagesObject, std::atomic_bool &is_recording_control);
  ~Recording();
  RecordingParameters parameters() const { return _parameters; }
  void evaluate(const Frame::ptr &frame);
  bool accepting_frames() const;
  void handle(const Frame::ptr &frame);
  
private:
  const RecordingParameters _parameters;
  LocalSaveImages *saveImagesObject;
  std::atomic_bool &is_recording_control;
  chrono::steady_clock::time_point started;
  fps_counter savefps, meanfps;
  FileWriter::Ptr file_writer;
  size_t frames = 0;
  Frame::ptr reference;
  QDateTime timelapse_last_shot;
};

class WriterThreadWorker : public QObject {
  Q_OBJECT
public:
  WriterThreadWorker ( LocalSaveImages *saveImages, QObject* parent = 0 );
  virtual ~WriterThreadWorker();
  void stop();
public slots:
  virtual void queue(const Frame::ptr &frame);
  void start(const RecordingParameters &recording, qlonglong max_memory_usage);
private:
  unique_ptr<FramesQueue> framesQueue;
  LocalSaveImages *saveImages;
  atomic_bool is_recording;
  size_t max_memory_usage;
  uint64_t dropped_frames;
};


}
Q_DECLARE_METATYPE(RecordingParameters)


Recording::Recording(const RecordingParameters &parameters, LocalSaveImages *saveImagesObject, std::atomic_bool &is_recording_control) : 
  _parameters{parameters},
  saveImagesObject{saveImagesObject},
  is_recording_control(is_recording_control),
  started{chrono::steady_clock::now()},
  savefps{[=](double fps){ emit saveImagesObject->saveFPS(fps);}, fps_counter::Elapsed},
  meanfps{[=](double fps){ emit saveImagesObject->meanFPS(fps);}, fps_counter::Elapsed, 1000, true},
  file_writer{parameters.fileWriterFactory()}
{
  is_recording_control = true;
  _parameters.recording_information->set_writer(_parameters.recording_information_writer(file_writer));
  emit saveImagesObject->recording(file_writer->filename());
}



void Recording::handle(const Frame::ptr &frame) {
  file_writer->handle(frame);
  ++savefps;
  ++meanfps;
  emit saveImagesObject->savedFrames(++frames);
}
void Recording::evaluate(const Frame::ptr &frame) {
  if(frames == 0) {
    reference = frame;
  }
  if(parameters().timelapse) {
    if(frames == 0 || (timelapse_last_shot.msecsTo(frame->created_utc()) >= parameters().timelapse_msecs )) {
      timelapse_last_shot = frame->created_utc();
      handle(frame);
    }
  } else {
    handle(frame);
  }
}

bool Recording::accepting_frames() const {
  return 
    is_recording_control && (
    _parameters.limit_type == Configuration::Infinite || 
    ( _parameters.limit_type == Configuration::FramesNumber && frames < _parameters.max_frames ) ||
    ( _parameters.limit_type == Configuration::Duration && chrono::steady_clock::now() - started < _parameters.max_seconds) ||
    ( _parameters.limit_type == Configuration::FileSize && (! reference || reference->size() * frames < _parameters.max_size) )
    );
}

Recording::~Recording() {
  if(reference)
    _parameters.recording_information->set_ended(frames, reference->resolution().width(), reference->resolution().height(), reference->bpp(), reference->channels());
  is_recording_control = false;
  emit saveImagesObject->finished();
}

WriterThreadWorker::WriterThreadWorker (LocalSaveImages *saveImages, QObject* parent )
  : QObject(parent), saveImages{saveImages}, is_recording{false}
{
    static bool metatypes_registered = false;
    if(!metatypes_registered) {
      metatypes_registered = true;
      qRegisterMetaType<RecordingParameters>();
    }
}

WriterThreadWorker::~WriterThreadWorker()
{
}

void WriterThreadWorker::stop()
{
  is_recording = false;
}


void WriterThreadWorker::queue(const Frame::ptr &frame)
{
  if(!is_recording)
    return;
  if(!framesQueue  ) {
    framesQueue.reset(new FramesQueue{ std::max(max_memory_usage/frame->size(), size_t{1})  } );
    qDebug() << "allocated framesqueue with " << max_memory_usage << " bytes capacity (" << max_memory_usage/frame->size()<< " frames)";
  }
  
  if(!framesQueue->push(frame)) {
    qWarning() << "Frames queue too high, dropping frame";
    emit saveImages->droppedFrames(++dropped_frames);
  }
}

void WriterThreadWorker::start(const RecordingParameters & recording_parameters, qlonglong max_memory_usage)
{
  this->max_memory_usage = static_cast<size_t>(max_memory_usage);
  dropped_frames = 0;
  framesQueue.reset();

  Recording recording{recording_parameters, saveImages, is_recording};
  
  while(recording.accepting_frames() ) {
    Frame::ptr frame;
    if(framesQueue && framesQueue->pop(frame)) {
      recording.evaluate(frame);
    } else {
      QThread::msleep(1);
    }
  }
}


FileWriter::Factory LocalSaveImages::Private::writerFactory()
{
  if(configuration->savefile().isEmpty()) {
    return {};
  }

  return FileWriter::factories()[configuration->save_format()];
}

LocalSaveImages::LocalSaveImages(const Configuration::ptr & configuration, QObject* parent)
  : dptr(configuration, new WriterThreadWorker(this), new QThread, this)
{
  d->worker->moveToThread(d->recordingThread);
  d->recordingThread->start();
}


LocalSaveImages::~LocalSaveImages()
{
  endRecording();
  d->recordingThread->quit();
  d->recordingThread->wait();
}


void LocalSaveImages::handle(const Frame::ptr &frame)
{
  d->worker->queue(frame);
//   QtConcurrent::run(bind(&WriterThreadWorker::handle, d->worker, frame));
}

void LocalSaveImages::startRecording(Imager *imager)
{
  auto writerFactory = d->writerFactory();
  if(writerFactory) {
    RecordingInformation::ptr recording_information;
    
    RecordingParameters recording{
      bind(writerFactory, imager->name(), d->configuration), 
      make_shared<RecordingInformation>(d->configuration, imager),
      d->configuration->recording_limit_type(),
      d->configuration->recording_frames_limit(),
      chrono::duration<double>{d->configuration->recording_seconds_limit()},
      d->configuration->save_info_file(),
      d->configuration->save_json_info_file(),
      0,
      d->configuration->timelapse_mode(),
      d->configuration->timelapse_msecs(),
      d->configuration
    };    
    QMetaObject::invokeMethod(d->worker, "start", Q_ARG(RecordingParameters, recording), Q_ARG(qlonglong, d->configuration->max_memory_usage() ));    
  }
}

void LocalSaveImages::setPaused(bool paused)
{
  // TODO
}




void LocalSaveImages::endRecording()
{
  if(d->worker)
    d->worker->stop();
}


#include "local_saveimages.moc"
