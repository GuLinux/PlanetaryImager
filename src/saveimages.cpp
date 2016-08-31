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
#include "opencv_utils.h"
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

DPTR_IMPL(SaveImages) {
    Configuration &configuration;
    WriterThreadWorker *worker;
    QThread *recordingThread;
    SaveImages *q;
    FileWriter::Factory writerFactory();
};


namespace {
typedef boost::lockfree::spsc_queue<Frame::ptr> FramesQueue;
typedef function< FileWriter::Ptr() > CreateFileWriter;
struct Recording;
  
class WriterThreadWorker : public QObject {
  Q_OBJECT
public:
  WriterThreadWorker ( SaveImages *saveImages, QObject* parent = 0 );
  virtual ~WriterThreadWorker();
  void stop();
public slots:
  virtual void handle(const Frame::ptr &frame);
  void start(const Recording &recording, qlonglong max_memory_usage);
private:
  unique_ptr<FramesQueue> framesQueue;
  SaveImages *saveImages;
  atomic_bool is_recording;
  size_t max_memory_usage;
  uint64_t dropped_frames;
};

struct Recording {
    CreateFileWriter fileWriterFactory;
  long long max_frames;
  RecordingInformation::ptr recording_information;
};
}
Q_DECLARE_METATYPE(Recording)


WriterThreadWorker::WriterThreadWorker (SaveImages *saveImages, QObject* parent )
  : QObject(parent), saveImages{saveImages}, is_recording{false}
{
    static bool metatypes_registered = false;
    if(!metatypes_registered) {
      metatypes_registered = true;
      qRegisterMetaType<Recording>();
    }
}

WriterThreadWorker::~WriterThreadWorker()
{
}

void WriterThreadWorker::stop()
{
  is_recording = false;
}


void WriterThreadWorker::handle(const Frame::ptr &frame)
{
  if(!is_recording)
    return;
  if(!framesQueue) {
    framesQueue.reset(new FramesQueue{ std::max(max_memory_usage/frame->size(), size_t{1})  } );
    qDebug() << "allocated framesqueue with " << max_memory_usage << " bytes capacity (" << max_memory_usage/frame->size()<< " frames)";
  }
  
  if(!framesQueue->push(frame)) {
    qWarning() << "Frames queue too high, dropping frame";
    emit saveImages->droppedFrames(++dropped_frames);
  }
}

void WriterThreadWorker::start(const Recording& recording, qlonglong max_memory_usage)
{
  this->max_memory_usage = static_cast<size_t>(max_memory_usage);
  dropped_frames = 0;
  is_recording = true;
  auto fileWriter = recording.fileWriterFactory();
  recording.recording_information->set_base_filename(fileWriter->filename());
  fps_counter savefps{[=](double fps){ emit saveImages->saveFPS(fps);}, fps_counter::Elapsed};
  fps_counter meanfps{[=](double fps){ emit saveImages->meanFPS(fps);}, fps_counter::Elapsed, 1000, true};
  uint64_t frames = 0;
  emit saveImages->recording(fileWriter->filename());
  Frame::ptr reference;
  GuLinux::Scope on_finish{[&]{
    is_recording = false;
    framesQueue.reset();
    recording.recording_information->set_ended(frames, reference->resolution().width(), reference->resolution().height(), reference->bpp(), reference->channels());
    emit saveImages->finished();
  }};
  while(is_recording && frames < recording.max_frames) {
    Frame::ptr frame;
    if(framesQueue && framesQueue->pop(frame)) {
      fileWriter->handle(frame);
      ++savefps;
      ++meanfps;
      emit saveImages->savedFrames(++frames);
      if(! reference) {
        reference = frame;
      }
    } else {
      QThread::msleep(1);
    }
  }
}


FileWriter::Factory SaveImages::Private::writerFactory()
{
  if(configuration.savefile().isEmpty()) {
    return {};
  }

  return FileWriter::factories()[configuration.save_format()];
}

SaveImages::SaveImages(Configuration& configuration, QObject* parent) : QObject(parent), dptr(configuration, new WriterThreadWorker(this), new QThread, this)
{
  d->worker->moveToThread(d->recordingThread);
  d->recordingThread->start();
}


SaveImages::~SaveImages()
{
  endRecording();
  d->recordingThread->quit();
  d->recordingThread->wait();
}


void SaveImages::handle(const Frame::ptr &frame)
{
  d->worker->handle(frame);
//   QtConcurrent::run(bind(&WriterThreadWorker::handle, d->worker, frame));
}

void SaveImages::startRecording(Imager *imager)
{
  auto writerFactory = d->writerFactory();
  if(writerFactory) {
    RecordingInformation::ptr recording_information;
    if(d->configuration.save_info_file())
      recording_information = make_shared<RecordingInformation>(d->configuration, imager);
    
    Recording recording{
      bind(writerFactory, imager->name(), std::ref<Configuration>(d->configuration)), 
      d->configuration.recording_frames_limit() == 0 ? std::numeric_limits<long long>().max() : d->configuration.recording_frames_limit(),
      recording_information,
    };
    
    QMetaObject::invokeMethod(d->worker, "start", Q_ARG(Recording, recording), Q_ARG(qlonglong, d->configuration.max_memory_usage() ));    
  }
}



void SaveImages::endRecording()
{
  if(d->worker)
    d->worker->stop();
}


#include "saveimages.moc"
