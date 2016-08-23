/*
 * Copyright (C) 2016  <copyright holder> <email>
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

#ifndef CAMERAINFOWIDGET_H
#define CAMERAINFOWIDGET_H

#include <QWidget>
#include "c++/dptr.h"
#include <drivers/imager.h>

class CameraInfoWidget : public QWidget
{
  Q_OBJECT
public:
  CameraInfoWidget(Imager* imager, QWidget* parent = 0);
  virtual ~CameraInfoWidget();
private:
  DPTR
};

#endif // CAMERAINFOWIDGET_H
