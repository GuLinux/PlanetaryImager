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
#include "menucontrolwidget.h"

struct MenuControlWidget::Private {
  QComboBox *edit;
};

MenuControlWidget::MenuControlWidget(QWidget* parent): ControlWidget(parent), dptr()
{
  layout()->addWidget(d->edit = new QComboBox);
  connect(d->edit, F_PTR(QComboBox, currentIndexChanged, int), [=](int index) { emit valueChanged(d->edit->itemData(index).toDouble()); });
}

MenuControlWidget::~MenuControlWidget()
{
}

void MenuControlWidget::update(const Imager::Setting& setting)
{
//   if(d->edit->currentData().toDouble() == setting.value)
//     return;
  d->edit->clear();
  for(auto item: setting.choices) {
    d->edit->addItem(item.label, item.value);
  }
  d->edit->setCurrentIndex(d->edit->findData(setting.value));
}

