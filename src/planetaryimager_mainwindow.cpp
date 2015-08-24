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

#include "planetaryimager_mainwindow.h"
#include "drivers/driver.h"
#include "drivers/imager.h"
#include "ui_planetaryimager_mainwindow.h"
#include <functional>
#include "utils.h"
#include "statusbarinfowidget.h"
#include "saveimages.h"
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSettings>
#include <QThread>
#include <QFileDialog>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent>
#include "fps_counter.h"
#include "camerasettingswidget.h"
#include "configurationdialog.h"
#include "configuration.h"
#include <QThread>
#include <QMutex>
#include <QMessageBox>
#include "displayimage.h"
#include "recordingpanel.h"

#include "Qt/strings.h"

using namespace std;
using namespace std::placeholders;



class PlanetaryImagerMainWindow::Private {
public:
  Private(PlanetaryImagerMainWindow *q);
  shared_ptr<Ui::PlanetaryImagerMainWindow> ui;
  DriverPtr driver = make_shared<SupportedDrivers>();
  ImagerPtr imager;
  void rescan_devices();
  QSettings settings;
  Configuration configuration;
  void saveState();
  QGraphicsScene *scene;
  double zoom;
  StatusBarInfoWidget *statusbar_info_widget;
  shared_ptr<DisplayImage> displayImage;
  QThread displayImageThread;
  shared_ptr<SaveImages> saveImages;
  CameraSettingsWidget* cameraSettingsWidget;
  ConfigurationDialog *configurationDialog;
    RecordingPanel* recording_panel;
  
  void connectCamera(const Driver::CameraPtr &camera);
  void cameraDisconnected();
  void enableUIWidgets(bool cameraConnected);
private:
  PlanetaryImagerMainWindow *q;
};

PlanetaryImagerMainWindow::Private::Private(PlanetaryImagerMainWindow* q) : ui{make_shared<Ui::PlanetaryImagerMainWindow>()}, settings{"GuLinux", qApp->applicationName()}, configuration{settings}, q{q}
{
}


PlanetaryImagerMainWindow::~PlanetaryImagerMainWindow()
{
  if(d->imager)
    d->imager->stopLive();
}

void PlanetaryImagerMainWindow::Private::saveState()
{
  settings.setValue("dock_settings", q->saveState());
}


PlanetaryImagerMainWindow::PlanetaryImagerMainWindow(QWidget* parent, Qt::WindowFlags flags) : dptr(this)
{
    d->ui->setupUi(this);
    setWindowIcon(QIcon::fromTheme("planetary_imager"));
    d->ui->recording->setWidget(d->recording_panel = new RecordingPanel{d->configuration});
    d->configurationDialog = new ConfigurationDialog(d->configuration, this);
    d->displayImage = make_shared<DisplayImage>(d->configuration);
    d->saveImages = make_shared<SaveImages>(d->configuration);
    d->ui->statusbar->addPermanentWidget(d->statusbar_info_widget = new StatusBarInfoWidget());
    d->scene = new QGraphicsScene(this);
    restoreState(d->settings.value("dock_settings").toByteArray());
    connect(d->ui->actionAbout, &QAction::triggered, bind(&QMessageBox::about, this, tr("About"),
							  tr("%1 version %2.\nFast imaging capture software for planetary imaging").arg(qApp->applicationDisplayName())
							 .arg(qApp->applicationVersion())));
    connect(d->ui->actionAbout_Qt, &QAction::triggered, &QApplication::aboutQt);
    connect(d->ui->action_devices_rescan, &QAction::triggered, bind(&Private::rescan_devices, d.get()));
    connect(d->ui->actionShow_settings, &QAction::triggered, bind(&QDialog::show, d->configurationDialog));
    
    d->ui->image->setScene(d->scene);
    auto dockWidgetToggleVisibility = [=](QDockWidget *widget, bool visible){ widget->setVisible(visible); };
    auto dockWidgetVisibleCheck = [=](QAction *action, QDockWidget *widget) { action->setChecked(widget->isVisible()); };
    QList<QDockWidget*> dock_widgets;
    auto setupDockWidget = [&](QAction *action, QDockWidget *widget){
      dockWidgetVisibleCheck(action, widget);
      connect(action, &QAction::triggered, bind(dockWidgetToggleVisibility, widget, _1));
      connect(widget, &QDockWidget::visibilityChanged, bind(dockWidgetVisibleCheck, action, widget));
      connect(widget, &QDockWidget::dockLocationChanged, bind(&Private::saveState, d.get()));
      connect(widget, &QDockWidget::topLevelChanged, bind(&Private::saveState, d.get()));
      connect(widget, &QDockWidget::visibilityChanged, bind(&Private::saveState, d.get()));
      dock_widgets.push_back(widget);
    };
    d->zoom = 1;
    auto zoom = [=](qreal scale) { d->ui->image->scale(scale, scale); };
    connect(d->ui->actionZoom_In, &QAction::triggered, [=]{ zoom(1.05); });
    connect(d->ui->actionZoom_Out, &QAction::triggered, [=]{ zoom(0.95); });
    connect(d->ui->actionFit_to_window, &QAction::triggered, [=]{ d->ui->image->fitInView(d->displayImage->imageRect(), Qt::KeepAspectRatio); });
    connect(d->ui->actionActual_Size, &QAction::triggered, [=]{ d->ui->image->setTransform({}); });
    
    connect(d->recording_panel, &RecordingPanel::start, [=]{d->saveImages->startRecording(d->imager->name());});
    connect(d->recording_panel, &RecordingPanel::stop, bind(&SaveImages::endRecording, d->saveImages));
    
    connect(d->saveImages.get(), &SaveImages::recording, d->displayImage.get(), bind(&DisplayImage::setRecording, d->displayImage, true), Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::recording, d->recording_panel, bind(&RecordingPanel::recording, d->recording_panel, true, _1), Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::finished, d->recording_panel, bind(&RecordingPanel::recording, d->recording_panel, false, QString{}), Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::finished, d->displayImage.get(), [=]{
        QTimer::singleShot(5000,  bind(&DisplayImage::setRecording, d->displayImage, false));
    }, Qt::QueuedConnection);
    setupDockWidget(d->ui->actionChip_Info, d->ui->chipInfoWidget);
    setupDockWidget(d->ui->actionCamera_Settings, d->ui->camera_settings);
    setupDockWidget(d->ui->actionRecording, d->ui->recording);
    connect(d->ui->actionHide_all, &QAction::triggered, [=]{ for_each(begin(dock_widgets), end(dock_widgets), bind(&QWidget::hide, _1) ); });
    connect(d->ui->actionShow_all, &QAction::triggered, [=]{ for_each(begin(dock_widgets), end(dock_widgets), bind(&QWidget::show, _1) ); });
    
    d->rescan_devices();
    connect(d->displayImage.get(), &DisplayImage::gotImage, this, [=](const QImage &image) {
      d->scene->clear();
      d->scene->addPixmap(QPixmap::fromImage(image));
    }, Qt::QueuedConnection);

    
    connect(d->displayImage.get(), &DisplayImage::displayFPS, d->statusbar_info_widget, &StatusBarInfoWidget::displayFPS, Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::saveFPS, d->recording_panel, &RecordingPanel::saveFPS, Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::meanFPS, d->recording_panel, &RecordingPanel::meanFPS, Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::savedFrames, d->recording_panel, &RecordingPanel::saved, Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::droppedFrames, d->recording_panel, &RecordingPanel::dropped, Qt::QueuedConnection);
    connect(d->ui->actionDisconnect, &QAction::triggered, [=]{ d->imager.reset();});


    d->enableUIWidgets(false);

    d->saveImages->moveToThread(&d->displayImageThread);
    connect(&d->displayImageThread, &QThread::started, bind(&DisplayImage::create_qimages, d->displayImage));
    d->displayImageThread.start();
    d->ui->settings_container->setLayout(new QVBoxLayout);
    d->ui->settings_container->layout()->setSpacing(0);
    connect(qApp, &QApplication::aboutToQuit, this, [=]{ d->imager.reset(); }, Qt::QueuedConnection);
    connect(qApp, &QApplication::aboutToQuit, this, bind(&DisplayImage::quit, d->displayImage), Qt::QueuedConnection);
    d->ui->actionEdges_Detection->setChecked(d->configuration.detectEdges());
    d->displayImage->detectEdges(d->configuration.detectEdges());
    connect(d->ui->actionEdges_Detection, &QAction::toggled, [=](bool detect){
      d->configuration.detectEdges(detect);
      d->displayImage->detectEdges(detect);
    });
}


void PlanetaryImagerMainWindow::Private::rescan_devices()
{
  ui->menu_device_load->clear();
  future_run<Driver::Cameras>([=]{ return driver->cameras(); }, [=]( const Driver::Cameras &cameras){
    for(auto device: cameras) {
      auto message = tr("Found %1 devices").arg(cameras.size());
      qDebug() << message;
      ui->statusbar->showMessage(message, 10000);
      auto action = ui->menu_device_load->addAction(device->name());
      QObject::connect(action, &QAction::triggered, bind(&Private::connectCamera, this, device));
    }
  });
}

void PlanetaryImagerMainWindow::Private::connectCamera(const Driver::CameraPtr& camera)
{
  future_run<ImagerPtr>([=]{ return camera->imager(ImageHandlers{displayImage, saveImages}); }, [=](const ImagerPtr &imager){
    if(!imager)
      return;
    this->imager = imager;
    imager->startLive();
    statusbar_info_widget->deviceConnected(imager->name());
    connect(imager.get(), &Imager::disconnected, q, bind(&Private::cameraDisconnected, this), Qt::QueuedConnection);
    connect(imager.get(), &Imager::fps, statusbar_info_widget, &StatusBarInfoWidget::captureFPS, Qt::QueuedConnection);
    ui->camera_name->setText(imager->name());
    ui->camera_chip_size->setText(QString("%1x%2").arg(imager->chip().width, 2).arg(imager->chip().height, 2));
    ui->camera_bpp->setText("%1"_q % imager->chip().bpp);
    ui->camera_pixels_size->setText(QString("%1x%2").arg(imager->chip().pixelwidth, 2).arg(imager->chip().pixelheight, 2));
    ui->camera_resolution->setText(QString("%1x%2").arg(imager->chip().xres, 2).arg(imager->chip().yres, 2));
    ui->settings_container->layout()->addWidget(cameraSettingsWidget = new CameraSettingsWidget(imager, settings));
    enableUIWidgets(true);
  });
}


void PlanetaryImagerMainWindow::Private::cameraDisconnected()
{
  qDebug() << "camera disconnected";
  enableUIWidgets(false);
  ui->camera_name->clear();
  ui->camera_chip_size->clear();
  ui->camera_bpp->clear();
  ui->camera_pixels_size->clear();
  ui->camera_resolution->clear();
  delete cameraSettingsWidget;
  scene->clear();
  statusbar_info_widget->captureFPS(0);
}

void PlanetaryImagerMainWindow::Private::enableUIWidgets(bool cameraConnected)
{
  ui->actionZoom_In->setEnabled(cameraConnected);
  ui->actionZoom_Out->setEnabled(cameraConnected);
  ui->actionActual_Size->setEnabled(cameraConnected);
  ui->actionFit_to_window->setEnabled(cameraConnected);
  ui->actionDisconnect->setEnabled(cameraConnected);
  ui->recording->setEnabled(cameraConnected);
  ui->chipInfoWidget->setEnabled(cameraConnected);
  ui->camera_settings->setEnabled(cameraConnected);
}

#include "planetaryimager_mainwindow.moc"
