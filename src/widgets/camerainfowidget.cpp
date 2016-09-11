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
#include "ui_camerainfowidget.h"
#include "Qt/strings.h"

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(CameraInfoWidget) {
  Imager *imager;
  CameraInfoWidget *q;
  unique_ptr<Ui::CameraInfoWidget> ui;
};

CameraInfoWidget::CameraInfoWidget(Imager *imager, QWidget* parent): QWidget(parent), dptr(imager, this)
{
  d->ui = make_unique<Ui::CameraInfoWidget>();
  d->ui->setupUi(this);
  auto chip_text = [=](QLabel *text_label, const QString &text, const QList<int> &values){
    bool valid = all_of(begin(values), end(values), bind(greater<int>(), _1, 0));
    text_label->setText(valid ? text : "-");
  };
  d->ui->camera_name->setText(imager->name());
  auto chip = imager->chip();
  
  chip_text(d->ui->camera_chip_size, QString("%1x%2").arg(chip.width, 2).arg(chip.height, 2), {chip.width, chip.height});
  chip_text(d->ui->camera_pixels_size, QString("%1x%2").arg(chip.pixelwidth, 2).arg(chip.pixelheight, 2), {chip.pixelwidth, chip.pixelheight});
  
  chip_text(d->ui->camera_bpp, "%1"_q % chip.bpp, {chip.bpp});
  chip_text(d->ui->camera_resolution, "%1x%2"_q % chip.xres % chip.yres, {chip.xres, chip.yres});
  d->ui->chipInfo->setLayout(new QVBoxLayout);
  for(auto property: chip.properties) {
    qDebug() << "Property name: " << property.name << " = " << property.value;
    d->ui->chipInfo->layout()->addWidget(new QLabel("%1: %2"_q % property.displayName() % property.displayValue(), d->ui->chipInfo));
  }
}

CameraInfoWidget::~CameraInfoWidget()
{

}
