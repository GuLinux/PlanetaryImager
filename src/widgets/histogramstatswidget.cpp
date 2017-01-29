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

#include "histogramstatswidget.h"
#include "ui_histogramstatswidget.h"
using namespace std;

DPTR_IMPL(HistogramStatsWidget) {
  unique_ptr<Ui::HistogramStatsWidget> ui;
  HistogramStatsWidget *q;
};

HistogramStatsWidget::HistogramStatsWidget(Histogram::Channel channel, const QVariantMap& stats, QWidget *parent) : QGroupBox(parent), dptr(make_unique<Ui::HistogramStatsWidget>(), this)
{
  static QMap<Histogram::Channel, QString> names {
    {Histogram::Grayscale, "Grayscale"},
    {Histogram::Red, "Red"},
    {Histogram::Green, "Green"},
    {Histogram::Blue, "Blue"},
  };
  d->ui->setupUi(this);
  setTitle(tr("Statistics for channel %0").arg(names[channel]));
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
  d->ui->highlights->setText(format_stat("highlights"));
  d->ui->shadows->setText(format_stat("shadows"));
  d->ui->maximum->setText(QString("values: %1-%2, position: %3%")
  .arg(stats["maximum_value_min"].toInt() )
  .arg(stats["maximum_value_max"].toInt() )
  .arg(stats["maximum_value_percent"].toDouble(), 5, 'f', 1 )
  );
}

HistogramStatsWidget::~HistogramStatsWidget()
{
}
