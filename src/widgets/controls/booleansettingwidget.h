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
#ifndef BOOLEANSETTINGWIDGET_H
#define BOOLEANSETTINGWIDGET_H

#include <QCheckBox>
#include "settingwidget.h"
class BooleanSettingWidget : public SettingWidget {
  Q_OBJECT
public:
    BooleanSettingWidget(QWidget* parent = 0);
public slots:
  virtual void update(const Imager::Setting &setting);
private:
  QCheckBox *edit;
};
#endif // BOOLEANSETTINGWIDGET_H
