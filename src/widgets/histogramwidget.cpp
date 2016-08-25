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
#include "configuration.h"
#include "Qt/functional.h"

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(HistogramWidget) {
  Histogram::ptr histogram;
  Configuration &configuration;
  HistogramWidget *q;
  std::unique_ptr<Ui::HistogramWidget> ui;
  QCPBars *histogram_plot;
  void got_histogram(const vector< uint32_t >& histogram);
  void toggle_histogram(bool enabled);
};
HistogramWidget::~HistogramWidget()
{
  d->ui->histogram_plot->clearItems();
  d->ui->histogram_plot->clearGraphs();
}

HistogramWidget::HistogramWidget(const Histogram::ptr &histogram, Configuration &configuration, QWidget* parent) : dptr(histogram, configuration, this)
{
    d->ui.reset(new Ui::HistogramWidget);
    d->ui->setupUi(this);
    d->ui->histogram_bins->setValue(d->configuration.histogram_bins());
    
    auto update_bins = [&]{
      auto value = d->ui->histogram_bins->value();
      d->histogram->set_bins(value);
      d->configuration.set_histogram_bins(value);
    };
    update_bins();
    d->histogram_plot = new QCPBars(d->ui->histogram_plot->xAxis, d->ui->histogram_plot->yAxis);
    d->ui->histogram_plot->addPlottable(d->histogram_plot);
    connect(d->ui->histogram_bins, F_PTR(QSpinBox, valueChanged, int), update_bins);
    connect(d->histogram.get(), &Histogram::histogram, this, bind(&Private::got_histogram, d.get(), _1), Qt::QueuedConnection);
    connect(d->ui->enable_histogram, &QCheckBox::toggled, this, bind(&Private::toggle_histogram, d.get(), _1));
    d->toggle_histogram(configuration.histogram_enabled());
}

void HistogramWidget::Private::toggle_histogram(bool enabled)
{
  ui->enable_histogram->setChecked(enabled);
  ui->histogram_bins->setEnabled(enabled);
  ui->histogram_plot->setEnabled(enabled);
  histogram->setEnabled(enabled);
  if(!enabled) {
    histogram_plot->clearData();
    ui->histogram_plot->replot();
  }
  configuration.set_histogram_enabled(enabled);
}


void HistogramWidget::Private::got_histogram(const vector<uint32_t>& histogram)
{
//   ui->histogram_plot->graph(0)->clearData();
  QVector<double> x(histogram.size());
  QVector<double> y(histogram.size());
  std::iota(x.begin(), x.end(), 0);

  transform(histogram.begin(), histogram.end(), y.begin(), [](uint32_t i) { return static_cast<double>(i); });
  histogram_plot->clearData();
  histogram_plot->setData(x, y);
  histogram_plot->rescaleAxes();
  ui->histogram_plot->replot();
} 

