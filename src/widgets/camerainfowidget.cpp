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

#include "camerainfowidget.h"
#include "Qt/strings.h"
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(CameraInfoWidget) {
  Imager *imager;
  CameraInfoWidget *q;
  QGridLayout *layout;
  void addProperty(const Imager::Properties::Property &property);
};

CameraInfoWidget::CameraInfoWidget(Imager *imager, QWidget* parent): QWidget(parent), dptr(imager, this)
{
  d->layout = new QGridLayout(this);
  setLayout(d->layout);
  d->addProperty({"Name", QVariant{imager->name()}});
  auto properties = imager->properties().properties;
  properties.erase(remove_if(properties.begin(), properties.end(), [](const auto &p){ return p.hidden; }), properties.end());
  std::for_each(properties.begin(), properties.end(), bind(&Private::addProperty, d.get(), _1));
  d->layout->setRowStretch(d->layout->rowCount(), 1);
}

CameraInfoWidget::~CameraInfoWidget()
{
}

void CameraInfoWidget::Private::addProperty(const Imager::Properties::Property& property)
{
    int next_row = layout->rowCount();
    layout->addWidget(new QLabel(property.displayName()), next_row, 0);
    layout->addWidget(new QLabel(property.displayValue()), next_row, 1);
}

