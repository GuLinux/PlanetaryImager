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

#include <QDebug>
#include "Qt/strings.h"
#include "cvvideowriter.h"
using namespace std;

class cvVideoWriter::Private
{
public:
  Private ( const QString& deviceName, Configuration& configuration, cvVideoWriter* q );
  QString filename;
  Configuration &configuration;
  shared_ptr<cv::VideoWriter> videoWriter;
private:
  cvVideoWriter *q;
};

cvVideoWriter::Private::Private ( const QString& deviceName, Configuration& configuration, cvVideoWriter* q ) 
  : filename(configuration.savefile()), configuration{configuration}, q ( q )
{
}

cvVideoWriter::cvVideoWriter( const QString& deviceName, Configuration& configuration ) : dptr(deviceName, configuration, this)
{
  qDebug() << "writing to " << filename();
}

cvVideoWriter::~cvVideoWriter()
{
  d->videoWriter->release();
}

QString cvVideoWriter::filename() const
{
  return d->filename;
}

void cvVideoWriter::handle ( const cv::Mat& imageData )
{
  auto fourcc = [](const string &s) { return CV_FOURCC(s[0], s[1], s[2], s[3]); };
  try {
    if(!d->videoWriter)
      d->videoWriter = make_shared<cv::VideoWriter>(d->filename.toStdString(), fourcc("X264"), 25, cv::Size{imageData.cols, imageData.rows});
    if(!d->videoWriter || ! d->videoWriter->isOpened()) {
      qWarning() << "unable to open video file" << d->filename;
      return;
    }
    *d->videoWriter << imageData;
  } catch(cv::Exception &e) {
    qWarning() << "error on handle:" << e.msg << e.code << e.file << e.line << e.func;
  }
}
