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
#include "Qt/qt_strings_helper.h"
#include <opencv2/opencv.hpp>
#include <QDir>
#include <CCfits/CCfits>
#include "image_handlers/saveimages.h"

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(ImageFileWriter) {
  const Configuration &configuration;
  QString filename;
  ImageFileWriter *q;
  function<void(const Frame::ptr&)> writer;
  QString savename(const Frame::ptr &frame, const QString &extension) const;
  QDir savedir;
  void saveFITS(const Frame::ptr &frame) const;
  void saveCV(const Frame::ptr &frame, const QString &extension) const;
};

ImageFileWriter::ImageFileWriter(ImageFileWriter::Format format, const Configuration &configuration) : dptr(configuration, configuration.savefile(), this)
{
  qDebug() << "Format: " << format;
  switch(format) {
    case PNG:
      d->writer = bind(&Private::saveCV, d.get(), _1, "png");
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


void ImageFileWriter::doHandle(Frame::ptr frame)
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

/* Sample FITS header by INDI - try to add as much fields as possible
* SIMPLE  =                    T / file does conform to FITS standard
* BITPIX  =                   16 / number of bits per data pixel
* NAXIS   =                    2 / number of data axes
* NAXIS1  =                 1280 / length of data axis 1
* NAXIS2  =                 1024 / length of data axis 2
* EXTEND  =                    T / FITS dataset may contain extensions
* COMMENT   FITS (Flexible Image Transport System) format is defined in 'Astronomy
* COMMENT   and Astrophysics', volume 376, page 359; bibcode: 2001A&A...376..359H
* BZERO   =                32768 / offset data range to that of unsigned short
* BSCALE  =                    1 / default scaling factor
INSTRUME= 'CCD Simulator'      / CCD Name
* EXPTIME =                   4. / Total Exposure Time (s)
CCD-TEMP=                  20. / CCD Temperature (Celcius)
PIXSIZE1=     5.19999980926514 / Pixel Size 1 (microns)
PIXSIZE2=     5.19999980926514 / Pixel Size 2 (microns)
XBINNING=                    1 / Binning factor in width
YBINNING=                    1 / Binning factor in height
FRAME   = 'Light   '           / Frame Type
FILTER  = 'Red     '           / Filter
FOCALLEN=                1280. / Focal Length (mm)
* DATE-OBS= '2016-12-14T16:22:50.696' / UTC start date of observation
COMMENT Generated by INDI
END
*/
void ImageFileWriter::Private::saveFITS(const Frame::ptr& frame) const
{
  auto filename = savename(frame, "fits");
  if(frame->channels() != 1) {
    throw SaveImages::Error::openingFile(filename, QObject::tr("Colour images are currently unsupported for FITS writer"));
  }
  long naxes[2] = { frame->resolution().width(), frame->resolution().height() };
  try {
    CCfits::FITS fits{filename.toStdString(), frame->bpp() == 8 ? BYTE_IMG : USHORT_IMG, 2, naxes};
    valarray<long> data(frame->resolution().width() * frame->resolution().height());
    auto mat = frame->mat();
    if(frame->bpp() == 8) {
      copy(mat.begin<uint8_t>(), mat.end<uint8_t>(), begin(data));
    } else {
      copy(mat.begin<uint16_t>(), mat.end<uint16_t>(), begin(data));
    }
    if(frame->exposure() != Frame::Seconds::zero()) {
      fits.pHDU().addKey("EXPTIME", frame->exposure().count(), "Total Exposure Time (s)");
    }
    fits.pHDU().addKey("DATE-OBS", frame->created_utc().toString(Qt::ISODate).toStdString(), "UTC start date of observation");
    fits.pHDU().write(1, data.size(), data);
    fits.flush();
  }
  catch(const CCfits::FitsException &e) {
    throw SaveImages::Error(QString::fromStdString(e.message()));
  }
}

void ImageFileWriter::Private::saveCV(const Frame::ptr& frame, const QString& extension) const
{
  const QString filename = savename(frame, extension);
  try {
    if(!cv::imwrite(filename.toStdString(), frame->mat()))
      throw SaveImages::Error::openingFile(filename);
  } catch(const exception &e) {
    throw SaveImages::Error(QString::fromStdString(e.what()));
  }
}

