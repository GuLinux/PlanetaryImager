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

#include "imagefilewriter.h"
#include <functional>
#include "Qt/strings.h"
#include <opencv2/opencv.hpp>
#include <QDir>
#include <CCfits/CCfits>

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(ImageFileWriter) {
  Configuration &configuration;
  QString filename;
  ImageFileWriter *q;
  function<void(const Frame::ptr&)> writer;
  QString savename(const Frame::ptr &frame, const QString &extension) const;
  QDir savedir;
  void saveFITS(const Frame::ptr &frame) const;
};

ImageFileWriter::ImageFileWriter(ImageFileWriter::Format format, Configuration& configuration) : dptr(configuration, configuration.savefile(), this)
{
  qDebug() << "Format: " << format;
  switch(format) {
    case PNG:
      d->writer = [&](const Frame::ptr &frame){ cv::imwrite(d->savename(frame, "png").toStdString(), frame->mat()); };
      break;
    case FITS:
      d->writer = bind(&Private::saveFITS, d.get(), _1);
      break;
  }
  d->savedir.mkpath(d->filename);
  d->savedir.cd(d->filename);
}

ImageFileWriter::~ImageFileWriter()
{
  if(d->savedir.count() == 0) {
    d->savedir.cdUp();
    d->savedir.rmpath(d->filename);
  }
}


void ImageFileWriter::handle(const Frame::ptr& frame)
{
  if(d->writer)
    d->writer(frame);
}

QString ImageFileWriter::filename() const
{
  return d->filename;
}

QString ImageFileWriter::Private::savename(const Frame::ptr& frame, const QString& extension) const
{
  return "%1/%2.%3"_q % filename % frame->created_utc().toString("yyyy-MM-ddTHHmmss.zzz-UTC") % extension;
}

void ImageFileWriter::Private::saveFITS(const Frame::ptr& frame) const
{
  if(frame->channels() != 1) {
    qWarning() << "Colour images are currently unsupported for FITS writer";
    return;
  }
  long naxes[2] = { frame->resolution().width(), frame->resolution().height() };
  CCfits::FITS fits{savename(frame, "fits").toStdString(), frame->bpp() == 8 ? BYTE_IMG : USHORT_IMG, 2, naxes};
  valarray<long> data(frame->resolution().width() * frame->resolution().height());
  auto mat = frame->mat();
  if(frame->bpp() == 8) {
    copy(mat.begin<uint8_t>(), mat.end<uint8_t>(), begin(data));
  } else {
    copy(mat.begin<uint16_t>(), mat.end<uint16_t>(), begin(data));
  }
  fits.pHDU().write(1, data.size(), data);
  fits.flush();
}

