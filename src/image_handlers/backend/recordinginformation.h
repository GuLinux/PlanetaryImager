/*
 * Copyright (C) 2016 Marco Gulino (marco AT gulinux.net)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef RECORDINGINFORMATION_H
#define RECORDINGINFORMATION_H

#include "c++/dptr.h"
#include <QString>
#include <QVariantMap>
#include "commons/configuration.h"
#include "commons/fwd.h"


FWD(Imager)
FWD_PTR(RecordingInformation)

class RecordingInformation
{
public:
  class Writer {
  public:
    typedef std::shared_ptr<Writer> ptr;
    virtual void write(const QVariantMap &information) = 0;
  };
  RecordingInformation(const Configuration &configuration, Imager *imager);
  ~RecordingInformation();
  void set_writer(const Writer::ptr &writer);
  void set_ended(int total_frames, int width, int height, uint8_t bpp, uint8_t channels);
  static Writer::ptr json(const QString &file_base_name, Configuration &configuration);
  static Writer::ptr txt(const QString &file_base_name);
  static Writer::ptr composite(const QList<Writer::ptr> &writers);
private:
  DPTR
};

#endif // RECORDINGINFORMATION_H
