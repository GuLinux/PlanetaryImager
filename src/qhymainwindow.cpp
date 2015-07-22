/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#include "qhymainwindow.h"
#include "qhydriver.h"
#include "qhyccdimager.h"
#include "ui_qhymainwindow.h"
#include <functional>
#include "utils.h"

using namespace std;
using namespace std::placeholders;
class QHYMainWindow::Private {
public:
  Private(QHYMainWindow *q);
  shared_ptr<Ui::QHYMainWindow> ui;
  QHYDriver driver;
  void rescan_devices();
  shared_ptr<QHYCCDImager> imager;
private:
  QHYMainWindow *q;
};

QHYMainWindow::Private::Private(QHYMainWindow* q) : ui{make_shared<Ui::QHYMainWindow>()}, q{q}
{
}


QHYMainWindow::~QHYMainWindow()
{
}

QHYMainWindow::QHYMainWindow(QWidget* parent, Qt::WindowFlags flags) : dpointer(this)
{
    d->ui->setupUi(this);
    connect(d->ui->action_devices_rescan, &QAction::triggered, bind(&Private::rescan_devices, d.get()));
    
    auto dockWidgetToggleVisibility = [=](QDockWidget *widget, bool visible){ widget->setVisible(visible); };
    auto dockWidgetVisibleCheck = [=](QAction *action, QDockWidget *widget) { action->setChecked(widget->isVisible()); };
    auto setupDockWidget = [=](QAction *action, QDockWidget *widget){
      dockWidgetVisibleCheck(action, widget);
      connect(action, &QAction::triggered, bind(dockWidgetToggleVisibility, widget, _1));
      connect(widget, &QDockWidget::visibilityChanged, bind(dockWidgetVisibleCheck, action, widget));
    };
    
    setupDockWidget(d->ui->actionChip_Info, d->ui->chipInfoWidget);
    setupDockWidget(d->ui->actionCamera_Settings, d->ui->camera_settings);
    d->rescan_devices();
}


void QHYMainWindow::Private::rescan_devices()
{
  ui->menu_device_load->clear();
  for(auto device: driver.cameras()) {
    auto action = ui->menu_device_load->addAction(device.name());
    QObject::connect(action, &QAction::triggered, [=]{
      imager = make_shared<QHYCCDImager>(device);
      ui->camera_name->setText(imager->name());
      ui->camera_chip_size->setText(QString("%1x%2").arg(imager->chip().width, 2).arg(imager->chip().height, 2));
      ui->camera_bpp->setText("%1"_q % imager->chip().bpp);
      ui->camera_pixels_size->setText(QString("%1x%2").arg(imager->chip().pixelwidth, 2).arg(imager->chip().pixelheight, 2));
      ui->camera_resolution->setText(QString("%1x%2").arg(imager->chip().xres, 2).arg(imager->chip().yres, 2));
    });
  }
}
