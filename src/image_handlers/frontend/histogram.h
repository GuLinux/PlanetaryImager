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
class Histogram : public QObject, public ImageHandler
{
  Q_OBJECT
public:
  typedef std::shared_ptr<Histogram> ptr;
  ~Histogram();
  Histogram(const Configuration::ptr &configuration, QObject* parent = 0);
  virtual void handle(const Frame::ptr &frame);
  void set_bins(std::size_t bins_size);
  void setRecording(bool recording);
  void setLogarithmic(bool logarithmic);
public slots:
  void read_settings();
signals:
  void histogram(const QImage &);
private:
  DPTR
};

#endif // HISTOGRAM_H
