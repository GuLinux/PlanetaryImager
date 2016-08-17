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

#ifndef SETTINGWIDGET_H
#define SETTINGWIDGET_H
#include <QWidget>
#include "drivers/imager.h"
#include "Qt/functional.h"
#include <QLayout>
#include "c++/dptr.h"

class ControlWidget : public QWidget {
  Q_OBJECT
public:
  ControlWidget(QWidget* parent = 0);
public slots:
  virtual void update(const Imager::Control &setting) = 0;
signals:
  void valueChanged(double value);
};

#endif // SETTINGWIDGET_H
