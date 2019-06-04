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

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QtCore>
#include "image_handlers/imagehandler.h"
#include "dptr.h"
#include "commons/configuration.h"
#include "commons/fwd.h"

FWD_PTR(Histogram)

class Histogram : public QObject, public ImageHandler
{
  Q_OBJECT
public:
  enum Channel {
    Grayscale,
    Red,
    Green,
    Blue,
    All,
  };
  ~Histogram();
  Histogram(const Configuration &configuration, QObject* parent = 0);
  void set_bins(std::size_t bins_size);
  void setRecording(bool recording);
  void setLogarithmic(bool logarithmic);
  Channel channel() const;
public slots:
  void read_settings();
  void setChannel(Channel channel);
signals:
  void histogram(const QImage &, const QMap<Histogram::Channel, QVariantMap> &, Histogram::Channel channel);
private:

  void doHandle(FrameConstPtr frame) override;

  DPTR
};

#endif // HISTOGRAM_H
