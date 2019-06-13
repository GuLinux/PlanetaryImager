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
#include "numbercontrolwidget.h"
#include <QDoubleSpinBox>
#include "Qt/qt_functional.h"

struct NumberControlWidget::Private {
  QDoubleSpinBox *edit;
};

NumberControlWidget::NumberControlWidget(QWidget* parent): ControlWidget(parent), dptr()
{
  layout()->addWidget(d->edit = new QDoubleSpinBox);
  connect(d->edit, F_PTR(QDoubleSpinBox, valueChanged, double), this, &ControlWidget::valueChanged);
}

NumberControlWidget::~NumberControlWidget()
{
}

void NumberControlWidget::update(const Imager::Control& setting)
{
  d->edit->setDecimals(setting.decimals);
  d->edit->setMinimum(setting.range.min.toDouble()); // TODO: depending on type?
  d->edit->setMaximum(setting.range.max.toDouble());
  d->edit->setSingleStep(setting.range.step.toDouble() != 0 ? setting.range.step.toDouble() : 0.1);
  d->edit->setValue(setting.value.toDouble());
}

QVariant NumberControlWidget::value() const
{
  return {d->edit->value()};
}
