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

#ifndef IMAGEFILEWRITER_H
#define IMAGEFILEWRITER_H
#include "filewriter.h"
#include "c++/dptr.h"
#include "commons/configuration.h"

class ImageFileWriter : public FileWriter
{
public:
  enum Format {PNG, FITS};
  ImageFileWriter(Format format, const Configuration &configuration);
  QString filename() const override;
  ~ImageFileWriter();
private:

    void doHandle(FrameConstPtr frame) override;

  DPTR
};

#endif // IMAGEFILEWRITER_H
