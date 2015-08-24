/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#ifndef DISPLAYIMAGE_H
#define DISPLAYIMAGE_H

#include <QObject>
#include "dptr.h"
#include "imagehandler.h"

class Configuration;
class DisplayImage : public QObject, public ImageHandler
{
Q_OBJECT
public:
    ~DisplayImage();
    DisplayImage(Configuration &configuration, QObject* parent = 0);
    virtual void handle(const ImageDataPtr& imageData);
    void setRecording(bool recording);
    QRect imageRect() const;
signals:
  void gotImage(const QImage &);
  void displayFPS(double fps);
public slots:
  void create_qimages();
//   void create_qimages_qt55();
  void quit();
private:
  D_PTR
};

#endif // DISPLAYIMAGE_H
