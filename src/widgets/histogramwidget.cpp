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
#include <opencv2/opencv.hpp>

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(HistogramWidget) {
  Histogram::ptr histogram;
  Configuration::ptr configuration;
  HistogramWidget *q;
  std::unique_ptr<Ui::HistogramWidget> ui;
  void got_histogram(const QImage& histogram, const QVariantMap &stats);
  void toggle_histogram_logarithmic(bool logarithmic);
};
HistogramWidget::~HistogramWidget()
{
}

HistogramWidget::HistogramWidget(const Histogram::ptr &histogram, const Configuration::ptr &configuration, QWidget* parent) : dptr(histogram, configuration, this)
{
    d->ui.reset(new Ui::HistogramWidget);
    d->ui->setupUi(this);
    d->ui->histogram_bins->setValue(d->configuration->histogram_bins());
    
    auto update_bins = [&]{
      auto value = d->ui->histogram_bins->value();
      d->histogram->set_bins(value);
      d->configuration->set_histogram_bins(value);
    };
    update_bins();
    connect(d->ui->histogram_bins, F_PTR(QSpinBox, valueChanged, int), update_bins);
    connect(d->histogram.get(), &Histogram::histogram, this, bind(&Private::got_histogram, d.get(), _1, _2), Qt::QueuedConnection);
    connect(d->ui->histogram_logarithmic, &QCheckBox::toggled, this, bind(&Private::toggle_histogram_logarithmic, d.get(), _1));
    d->toggle_histogram_logarithmic(configuration->histogram_logarithmic());
}

void HistogramWidget::Private::toggle_histogram_logarithmic(bool logarithmic)
{
  ui->histogram_logarithmic->setChecked(logarithmic);
  configuration->set_histogram_logarithmic(logarithmic);
  histogram->setLogarithmic(logarithmic);
}


void HistogramWidget::Private::got_histogram(const QImage& histogram, const QVariantMap& stats)
{
  ui->histogram_plot->setPixmap(QPixmap::fromImage(histogram));
  int totalPixels = stats["pixels"].toInt();
  int range_min = stats["range_min"].toInt();
  int range_max = stats["range_max"].toInt();
  auto format_stat = [totalPixels, range_min, range_max, &stats](const QString &name){
    int value = stats[name + "_value"].toInt();
    int pixels = stats[name + "_count"].toInt();
    double percent = 100. * pixels / totalPixels;
    double positionPercent = 100. * (value - range_min) / (range_max-range_min);
    return QString("value: %1, count: %2 (%3%), position: %4%")
      .arg(value, 5)
      .arg(pixels, 10)
      .arg(percent, 5, 'f', 1)
      .arg(positionPercent, 5, 'f', 1)
      ;
  };
  ui->highlights->setText(format_stat("highlights"));
  ui->shadows->setText(format_stat("shadows"));
  ui->maximum->setText(QString("value: %1-%2, position: %3%")
    .arg(stats["maximum_value_min"].toInt() )
    .arg(stats["maximum_value_max"].toInt() )
    .arg(stats["maximum_value_percent"].toDouble(), 5, 'f', 1 )
  );
} 

