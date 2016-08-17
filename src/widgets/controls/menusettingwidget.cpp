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
#include "menusettingwidget.h"


MenuSettingWidget::MenuSettingWidget(QWidget* parent): SettingWidget(parent)
{
  layout()->addWidget(edit = new QComboBox);
  connect(edit, F_PTR(QComboBox, currentIndexChanged, int), [=](int index) { emit valueChanged(edit->itemData(index).toDouble()); });
}

void MenuSettingWidget::update(const Imager::Setting& setting)
{
//   if(edit->currentData().toDouble() == setting.value)
//     return;
  edit->clear();
  for(auto item: setting.choices) {
    edit->addItem(item.label, item.value);
  }
  edit->setCurrentIndex(edit->findData(setting.value));
}

