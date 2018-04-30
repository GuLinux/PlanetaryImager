/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
 * Copyright (C) 2018  Filip Szczerek <ga.software@yahoo.com>
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

#include "mountwidget.h"


DPTR_IMPL(MountWidget)
{
};

MountWidget::~MountWidget()
{
}

MountWidget::MountWidget(QWidget* parent): QWidget(parent) //, dptr(histogram, configuration, this)
{
//    d->channel_combo_indexes = {
//      {Histogram::Grayscale, 0},
//      {Histogram::Red, 1},
//      {Histogram::Green, 2},
//      {Histogram::Blue, 3},
//      {Histogram::All, 4},
//    };
//    d->ui.reset(new Ui::HistogramWidget);
//    d->ui->setupUi(this);
//    d->ui->histogram_bins->setValue(d->configuration.histogram_bins());
//
//    auto update_bins = [&]{
//      auto value = d->ui->histogram_bins->value();
//      d->histogram->set_bins(value);
//      d->configuration.set_histogram_bins(value);
//    };
//    update_bins();
//    connect(d->ui->histogram_bins, F_PTR(QSpinBox, valueChanged, int), update_bins);
//    connect(d->histogram.get(), &Histogram::histogram, this, bind(&Private::got_histogram, d.get(), _1, _2, _3), Qt::QueuedConnection);
//    connect(d->ui->histogram_logarithmic, &QCheckBox::toggled, this, bind(&Private::toggle_histogram_logarithmic, d.get(), _1));
//    connect(d->ui->channel, F_PTR(QComboBox, currentIndexChanged, int), this, [this](int index) { d->histogram->setChannel(d->channel_combo_indexes.key(index)); });
//    d->toggle_histogram_logarithmic(configuration.histogram_logarithmic());
//    d->ui->statsWidget->setLayout(new QVBoxLayout);
}
