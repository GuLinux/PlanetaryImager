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
#include "numbersettingwidget.h"

NumberSettingWidget::NumberSettingWidget(QWidget* parent): SettingWidget(parent)
{
  layout()->addWidget(edit = new QDoubleSpinBox);
  connect(edit, F_PTR(QDoubleSpinBox, valueChanged, double), this, &SettingWidget::valueChanged);
}

void NumberSettingWidget::update(const Imager::Setting& setting)
{
  edit->setDecimals(setting.decimals);
  edit->setMinimum(setting.min);
  edit->setMaximum(setting.max);
  edit->setSingleStep(setting.step != 0 ? setting.step : 0.1);
  edit->setValue(setting.value);
}
