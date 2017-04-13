/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
 * Copyright (C) 2017  Marco Gulino <marco@gulinux.net>
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

#include "editroidialog.h"
#include "ui_editroidialog.h"
#include <QSize>
#include <QRect>
#include "Qt/strings.h"
#include "Qt/functional.h"
#include <QDebug>

using namespace std;


DPTR_IMPL(EditROIDialog) {
  unique_ptr<Ui::EditROIDialog> ui;
  EditROIDialog *q;
  QSize resolution;
  QRect roi;
  
  void updateWidgets();
  void startChanged();
  void sizeChanged();
  void endChanged();
};

EditROIDialog::EditROIDialog(QWidget *parent) : QDialog(parent), dptr(make_unique<Ui::EditROIDialog>(), this)
{
    d->ui->setupUi(this);
    // TODO: valueChanged should be more appropriate, in theory
    connect(d->ui->start_x, F_PTR(QSpinBox, editingFinished), this, bind(&Private::startChanged, d.get()));
    connect(d->ui->start_y, F_PTR(QSpinBox, editingFinished), this, bind(&Private::startChanged, d.get()));
    connect(d->ui->end_x, F_PTR(QSpinBox, editingFinished), this, bind(&Private::endChanged, d.get()));
    connect(d->ui->end_y, F_PTR(QSpinBox, editingFinished), this, bind(&Private::endChanged, d.get()));
    connect(d->ui->width, F_PTR(QSpinBox, editingFinished), this, bind(&Private::sizeChanged, d.get()));
    connect(d->ui->height, F_PTR(QSpinBox, editingFinished), this, bind(&Private::sizeChanged, d.get()));
    connect(this, &QDialog::accepted, this, [=]{ emit roiSelected(d->roi); });
}

EditROIDialog::~EditROIDialog()
{
}

void EditROIDialog::setCurrentROI(const QRect& roi)
{
}

void EditROIDialog::setResolution(const QSize& resolution)
{
  d->resolution = resolution;
  if(! d->roi.isValid())
    d->roi = {0, 0, d->resolution.width(), d->resolution.height()};
  d->updateWidgets();
}

void EditROIDialog::Private::updateWidgets()
{
  qDebug() << "ROI: " << roi << ", resolution: " << resolution;
  ui->resolution->setText("%1x%2"_q % resolution.width() % resolution.height());
  ui->start_x->setValue(roi.x());
  ui->start_y->setValue(roi.y());
  ui->width->setValue(roi.width());
  ui->height->setValue(roi.height());
  ui->end_x->setValue(roi.x() + roi.width() - 1);
  ui->end_y->setValue(roi.y() + roi.height() - 1);
  
  ui->start_x->setMaximum(resolution.width()-5);
  ui->start_y->setMaximum(resolution.height()-5);
  ui->width->setMaximum(resolution.width());
  ui->height->setMaximum(resolution.height());
  ui->end_x->setMaximum(resolution.width()-1);
  ui->end_y->setMaximum(resolution.height()-1);
}

void EditROIDialog::Private::startChanged()
{
  roi.setX(ui->start_x->value());
  roi.setY(ui->start_y->value());
  updateWidgets();
}

void EditROIDialog::Private::sizeChanged()
{
  roi.setWidth(ui->width->value());
  roi.setHeight(ui->height->value());
  updateWidgets();
}

void EditROIDialog::Private::endChanged()
{
  roi.setWidth(ui->end_x->value() - roi.x() + 1);
  roi.setHeight(ui->end_y->value() - roi.y() + 1);
  updateWidgets();
}

