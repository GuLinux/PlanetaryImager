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
#ifndef FILE_WRITER_H
#define FILE_WRITER_H
#include "image_handlers/imagehandler.h"
#include <QMap>
#include <memory>
#include <functional>
#include <QString>
#include "commons/configuration.h"

class FileWriter : public ImageHandler {
public:
  typedef std::shared_ptr<FileWriter> Ptr;
  typedef std::function<Ptr(const QString &deviceName, const Configuration::ptr &configuration)> Factory;
  virtual void handle(const Frame::ptr &frame) = 0;
  virtual QString filename() const = 0;
  static QMap<Configuration::SaveFormat, Factory> factories();
};


#endif
