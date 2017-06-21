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

#include "histogramwidget.h"
#include "ui_histogramwidget.h"
#include "commons/configuration.h"
#include "Qt/functional.h"
#include <QMap>
#include <opencv2/opencv.hpp>
#include "histogramstatswidget.h"

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(HistogramWidget) {
  Histogram::ptr histogram;
  Configuration  &configuration;
  HistogramWidget *q;
  std::unique_ptr<Ui::HistogramWidget> ui;
  void got_histogram(const QImage& histogram, const QMap<Histogram::Channel, QVariantMap> &stats, Histogram::Channel channel);
  void toggle_histogram_logarithmic(bool logarithmic);
  QMap<Histogram::Channel, int> channel_combo_indexes;
  QList<shared_ptr<HistogramStatsWidget>> statsWidgets;
};
HistogramWidget::~HistogramWidget()
{
}

HistogramWidget::HistogramWidget(const Histogram::ptr &histogram, Configuration &configuration, QWidget* parent) : QWidget(parent), dptr(histogram, configuration, this)
{
    d->channel_combo_indexes = {
      {Histogram::Grayscale, 0},
      {Histogram::Red, 1},
      {Histogram::Green, 2},
      {Histogram::Blue, 3},
      {Histogram::All, 4},
    };
    d->ui.reset(new Ui::HistogramWidget);
    d->ui->setupUi(this);
    d->ui->histogram_bins->setValue(d->configuration.histogram_bins());
    
    auto update_bins = [&]{
      auto value = d->ui->histogram_bins->value();
      d->histogram->set_bins(value);
      d->configuration.set_histogram_bins(value);
    };
    update_bins();
    connect(d->ui->histogram_bins, F_PTR(QSpinBox, valueChanged, int), update_bins);
    connect(d->histogram.get(), &Histogram::histogram, this, bind(&Private::got_histogram, d.get(), _1, _2, _3), Qt::QueuedConnection);
    connect(d->ui->histogram_logarithmic, &QCheckBox::toggled, this, bind(&Private::toggle_histogram_logarithmic, d.get(), _1));
    connect(d->ui->channel, F_PTR(QComboBox, currentIndexChanged, int), this, [this](int index) { d->histogram->setChannel(d->channel_combo_indexes.key(index)); });
    d->toggle_histogram_logarithmic(configuration.histogram_logarithmic());
    d->ui->statsWidget->setLayout(new QVBoxLayout);
}

void HistogramWidget::Private::toggle_histogram_logarithmic(bool logarithmic)
{
  ui->histogram_logarithmic->setChecked(logarithmic);
  configuration.set_histogram_logarithmic(logarithmic);
  histogram->setLogarithmic(logarithmic);
}


void HistogramWidget::Private::got_histogram(const QImage& histogram, const QMap<Histogram::Channel, QVariantMap>& stats, Histogram::Channel channel)
{
  ui->channel->setCurrentIndex(channel_combo_indexes[channel]);
  ui->histogram_plot->setPixmap(QPixmap::fromImage(histogram));
  statsWidgets.clear();
  for(auto channel: stats.keys()) {
    auto widget = make_shared<HistogramStatsWidget>(channel, stats[channel], ui->statsWidget);
    ui->statsWidget->layout()->addWidget(widget.get());
    statsWidgets.push_back(widget);
  }
} 

