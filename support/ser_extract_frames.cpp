#include "src/commons/ser_header.h"
#include "GuLinux-Commons/Qt/strings.h"

#include <iostream>
#include <QFile>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QRegularExpression>

#include <QDebug>
int help(const QString &message = {}) {
  QTextStream err(stderr);
  if(!message.isEmpty())
    err << message << endl;
  err << "Usage: ser_extract_frames input_filename output_filename <frames>" << endl;
  return 1;
}

using namespace std;
int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  QStringList args = app.arguments();
  args.removeFirst();
  if(args.size() < 3)
    return help();
  QFile input(args.takeFirst());
  if(!input.exists())
    return help("file not existing");
  QFile output(args.takeFirst());
  if(output.exists())
    return help("Output file %1 already exists"_q % output.fileName());
  QList<int64_t> frames;
  auto isNumber = [=](const QString &n) { return QRegularExpression("^\\d+$").match(n).hasMatch(); };
  while(args.size() > 0) {
    QString frame = args.takeFirst();
    if(! frame.contains("-") && isNumber(frame)) {
      frames.push_back(frame.toLongLong());
    } else if(frame.contains("-")) {
      auto frames_range = frame.split("-");
      if(frames_range.size() > 1 && isNumber(frames_range[0]) && isNumber(frames_range[1])) {
	auto begin = frames_range[0].toLongLong();
	auto end = frames_range[1].toLongLong();
	if(begin < end) {
	  for(long long i = begin; i <= end; i++) {
	    frames.push_back(i);
	  }
	}
      }
    }
  }
  qSort(frames);
  if(!input.open(QIODevice::ReadOnly))
    return help("unable to open input file %1"_q % input.fileName());
  SER_Header *input_header = reinterpret_cast<SER_Header*>(input.map(0, sizeof(SER_Header)));
  if(input_header == 0 || input_header->frames <= 0)
    return help("Unable to open input file %1, perhaps is an invalid SER file?"_q % input.fileName());
  qDebug() << "input frames: " << input_header->frames;
  qDebug() << "frames: " << frames;
  if(!output.open(QIODevice::ReadWrite))
    return help("Unable to open output file %1 for writing");
  
  SER_Header output_header = *input_header;
  output_header.frames = frames.size();
  
  output.write(reinterpret_cast<char*>(&output_header), sizeof(SER_Header));
  int planes = (output_header.colorId <= SER_Header::BAYER_MYYC  ? 1 : 3); // TODO: verify
  size_t frame_size = output_header.imageWidth * output_header.imageHeight * planes * (output_header.pixelDepth <= 8 ? 1 : 2);
  auto seek_to = [&] (int64_t frame) {
    input.seek(sizeof(SER_Header) + (frame-1) * frame_size);
  };
  int current = 0;
  for(auto frame: frames) {
    seek_to(frame);
    qDebug() << "writing frame " << frame << "(" << ++current << "of" << frames.size() << ")";
    output.write(input.read(frame_size));
  }
  seek_to(input_header->frames+1);
  auto footer_ba = input.readAll();
  if(footer_ba.size() / 8 != input_header->frames) {
    qDebug() << "found invalid or not existing footer, skipping";
    return 0;
  }
  for(auto frame: frames) {
    output.write(footer_ba.mid(sizeof(SER_Timestamp)*(frame-1), sizeof(SER_Timestamp)));
  }
  return 0;
}
