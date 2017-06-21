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

#include <QDebug>
#include <QObject>
#include "Qt/strings.h"
#include "cvvideowriter.h"
#include "commons/opencv_utils.h"
#include "image_handlers/saveimages.h"
using namespace std;

DPTR_IMPL(cvVideoWriter) {
  QString filename;
  const Configuration &configuration;
  cvVideoWriter *q;
  cv::VideoWriter videoWriter;
};

cvVideoWriter::cvVideoWriter(const Configuration &configuration ) : dptr(configuration.savefile(), configuration, this)
{
  qDebug() << "writing to " << filename();
}

cvVideoWriter::~cvVideoWriter()
{
  d->videoWriter.release();
}

QString cvVideoWriter::filename() const
{
  return d->filename;
}

void cvVideoWriter::doHandle(Frame::ptr frame)
{
  auto fourcc = [](const string &s) { return CV_FOURCC(s[0], s[1], s[2], s[3]); };
  auto size = cv::Size{frame->mat().cols, frame->mat().rows};
  try {
    if(!d->videoWriter.isOpened())
      d->videoWriter.open(d->filename.toStdString(), fourcc(d->configuration.video_codec().toStdString()), 25, size);
    if(! d->videoWriter.isOpened()) {
      throw SaveImages::Error::openingFile(d->filename, QObject::tr("Check that output directory exists, and that the selected video encoder is supported by your system."));
    }
    d->videoWriter << frame->mat();
  } catch(cv::Exception &e) {
    qWarning() << "error on handle:" << e.msg << e.code << e.file << e.line << e.func;
  }
}
