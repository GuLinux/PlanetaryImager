/*
 * Copyright (C) 2016  Marco Gulino <marco@gulinux.net>
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
#include "widgets/statusbarinfowidget.h"
#include "saveimages.h"
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSettings>
#include <QThread>
#include <QFileDialog>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent>
#include "fps_counter.h"
#include "widgets/cameracontrolswidget.h"
#include "configurationdialog.h"
#include "configuration.h"
#include <QThread>
#include <QMutex>
#include <QMessageBox>
#include "displayimage.h"
#include "widgets/recordingpanel.h"
#include "widgets/histogram.h"
#include "Qt/zoomableimage.h"
#include <QGridLayout>
#include <QToolBar>
#include "Qt/strings.h"
#include <Qt/functional.h>

using namespace GuLinux;
using namespace std;
using namespace std::placeholders;


Q_DECLARE_METATYPE(cv::Mat)

DPTR_IMPL(PlanetaryImagerMainWindow) {
  PlanetaryImagerMainWindow *q;
  
  Private(PlanetaryImagerMainWindow *q);
  shared_ptr<Ui::PlanetaryImagerMainWindow> ui;
  DriverPtr driver = make_shared<SupportedDrivers>();
  Imager *imager = nullptr;
  void rescan_devices();
  QSettings settings;
  Configuration configuration;
  void saveState();

  StatusBarInfoWidget *statusbar_info_widget;
  shared_ptr<DisplayImage> displayImage;
  QThread displayImageThread;
  shared_ptr<SaveImages> saveImages;
  shared_ptr<Histogram> histogram;
  CameraControlsWidget* cameraSettingsWidget = nullptr;
  ConfigurationDialog *configurationDialog;
    
  RecordingPanel* recording_panel;
  
  void connectCamera(const Driver::CameraPtr &camera);
  void cameraDisconnected();
  void enableUIWidgets(bool cameraConnected);
    void init_devices_watcher();
  ZoomableImage *image;
  QCPBars *histogram_plot;
  void got_histogram(const vector< uint32_t >& histogram);
  QQueue<Imager::Control> settings_to_save_queue;
  
  QFutureWatcher<ImagerPtr> imager_future_watcher;
  void onImagerInitialized(const ImagerPtr &imager);
};

PlanetaryImagerMainWindow::Private::Private(PlanetaryImagerMainWindow* q) 
  : ui{make_shared<Ui::PlanetaryImagerMainWindow>()},
  settings{"GuLinux", qApp->applicationName()}, 
  configuration{settings}, 
  q{q}
{
}


PlanetaryImagerMainWindow::~PlanetaryImagerMainWindow()
{
  LOG_F_SCOPE
  d->ui->histogram_plot->clearItems();
  d->ui->histogram_plot->clearGraphs();
  d->ui.reset();
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
    d->histogram = make_shared<Histogram>();
    d->ui->histogram_bins->setValue(d->settings.value("histogram-bins", 50).toInt());
    
    auto update_bins = [&]{
      auto value = d->ui->histogram_bins->value();
      d->histogram->set_bins(value);
      d->settings.setValue("histogram-bins", value);
    };
    update_bins();
    connect(d->ui->histogram_bins, F_PTR(QSpinBox, valueChanged, int), update_bins);
    d->ui->statusbar->addPermanentWidget(d->statusbar_info_widget = new StatusBarInfoWidget(), 1);

    d->ui->image->setLayout(new QGridLayout);
    d->ui->image->layout()->setMargin(0);
    d->ui->image->layout()->setSpacing(0);
    d->ui->image->layout()->addWidget(d->image = new ZoomableImage(false));
    for(auto item: d->image->actions())
      d->ui->menuView->insertAction(d->ui->actionEdges_Detection, item);
    d->ui->menuView->insertSeparator(d->ui->actionEdges_Detection);
    
    d->image->actions()[ZoomableImage::Actions::ZoomIn]->setShortcut({Qt::CTRL + Qt::Key_Plus});
    d->image->actions()[ZoomableImage::Actions::ZoomOut]->setShortcut({Qt::CTRL + Qt::Key_Minus});
    d->image->actions()[ZoomableImage::Actions::ZoomFit]->setShortcut({Qt::CTRL + Qt::Key_Space});
    d->image->actions()[ZoomableImage::Actions::ZoomRealSize]->setShortcut({Qt::CTRL + Qt::Key_Backspace});
    
    addToolBar(d->image->toolbar());
    d->image->toolbar()->setFloatable(true);
    d->image->toolbar()->setMovable(true);
    
    restoreState(d->settings.value("dock_settings").toByteArray());
    connect(d->ui->actionAbout, &QAction::triggered, bind(&QMessageBox::about, this, tr("About"),
							  tr("%1 version %2.\nFast imaging capture software for planetary imaging").arg(qApp->applicationDisplayName())
							 .arg(qApp->applicationVersion())));
    connect(d->ui->actionAbout_Qt, &QAction::triggered, &QApplication::aboutQt);
    connect(d->ui->action_devices_rescan, &QAction::triggered, bind(&Private::rescan_devices, d.get()));
    connect(d->ui->actionShow_settings, &QAction::triggered, bind(&QDialog::show, d->configurationDialog));
    
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

    connect(d->recording_panel, &RecordingPanel::start, [=]{d->saveImages->startRecording(d->imager);});
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
    setupDockWidget(d->ui->actionHistogram, d->ui->histogram);
    connect(d->ui->actionHide_all, &QAction::triggered, [=]{ for_each(begin(dock_widgets), end(dock_widgets), bind(&QWidget::hide, _1) ); });
    connect(d->ui->actionShow_all, &QAction::triggered, [=]{ for_each(begin(dock_widgets), end(dock_widgets), bind(&QWidget::show, _1) ); });
    
    d->rescan_devices();
    connect(d->displayImage.get(), &DisplayImage::gotImage, this, bind(&ZoomableImage::setImage, d->image, _1), Qt::QueuedConnection);
    connect(d->histogram.get(), &Histogram::histogram, this, bind(&Private::got_histogram, d.get(), _1), Qt::QueuedConnection);

    
    connect(d->displayImage.get(), &DisplayImage::displayFPS, d->statusbar_info_widget, &StatusBarInfoWidget::displayFPS, Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::saveFPS, d->recording_panel, &RecordingPanel::saveFPS, Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::meanFPS, d->recording_panel, &RecordingPanel::meanFPS, Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::savedFrames, d->recording_panel, &RecordingPanel::saved, Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::droppedFrames, d->recording_panel, &RecordingPanel::dropped, Qt::QueuedConnection);
    connect(d->ui->actionDisconnect, &QAction::triggered, [=]{ 
      LOG_F_SCOPE 
      d->imager->stopLive();
      d->imager->deleteLater();
      QTimer::singleShot(500, bind(&Private::cameraDisconnected, d.get()));
    });

    d->enableUIWidgets(false);

    d->saveImages->moveToThread(&d->displayImageThread);
    connect(&d->displayImageThread, &QThread::started, bind(&DisplayImage::create_qimages, d->displayImage));
    d->displayImageThread.start();

    connect(qApp, &QApplication::aboutToQuit, this, [=]{
      if(d->imager)
        d->imager->stopLive();
    }, Qt::QueuedConnection);
    connect(qApp, &QApplication::aboutToQuit, this, [&] {
      d->displayImage->quit();
      d->displayImageThread.quit();
      d->displayImageThread.wait();
    });
    connect(d->ui->actionEdges_Detection, &QAction::toggled, [=](bool detect){
      d->displayImage->detectEdges(detect);
    });
    d->init_devices_watcher();
    d->histogram_plot = new QCPBars(d->ui->histogram_plot->xAxis, d->ui->histogram_plot->yAxis);
    d->ui->histogram_plot->addPlottable(d->histogram_plot);
    connect(d->ui->actionClear_ROI, &QAction::triggered, [&] { d->imager->clearROI(); });
    connect(d->ui->actionSelect_ROI, &QAction::triggered, [&] { d->image->startSelectionMode(); });
    connect(d->image, &ZoomableImage::selectedROI, [&](const QRectF &rect) {  // TODO: safety check if we add more selection modes other than ROI
      d->imager->setROI(rect.toRect());
      d->image->clearROI();
    });
    connect(&d->imager_future_watcher, &QFutureWatcher<ImagerPtr>::finished, this, [&]{
      d->onImagerInitialized(d->imager_future_watcher.result());
    });
}

#include <iostream>

void PlanetaryImagerMainWindow::Private::init_devices_watcher()
{
#ifdef Q_OS_LINUX
  auto notifyTimer = new QTimer(q);
  QString usbfsdir;
  for(auto path: QStringList{"/proc/bus/usb/devices", "/sys/bus/usb/devices"}) {
    if(QDir(path).exists())
      usbfsdir = path;
  }
  if(usbfsdir.isEmpty())
    return;
  connect(notifyTimer, &QTimer::timeout, [=]{
    static QStringList entries;
    auto current = QDir(usbfsdir).entryList();
    if(current != entries) {
      qDebug() << "usb devices changed";
      entries = current;
      rescan_devices();
    }
  });
  notifyTimer->start(1500);
#endif
}


void PlanetaryImagerMainWindow::Private::rescan_devices()
{
  ui->menu_device_load->clear();
  Thread::Run<Driver::Cameras>([=]{ return driver->cameras(); }, [=]( const Driver::Cameras &cameras){
    for(auto device: cameras) {
      auto message = tr("Found %1 devices").arg(cameras.size());
      qDebug() << message;
      statusbar_info_widget->showMessage(message, 10'000);
      auto action = ui->menu_device_load->addAction(device->name());
      QObject::connect(action, &QAction::triggered, bind(&Private::connectCamera, this, device));
    }
  });
}

void PlanetaryImagerMainWindow::Private::connectCamera(const Driver::CameraPtr& camera)
{
  imager_future_watcher.setFuture(QtConcurrent::run([=]{ return camera->imager(ImageHandlerPtr{new ImageHandlers{displayImage, saveImages, histogram}}); }));
}

void PlanetaryImagerMainWindow::Private::onImagerInitialized(const ImagerPtr& imager)
{
    auto chip_text = [=](QLabel *text_label, const QString &text, const QList<int> &values){
        bool valid = all_of(begin(values), end(values), bind(greater<int>(), _1, 0));
        text_label->setText(valid ? text : "-");
    };
  
    if(!imager) {
      for(auto widget: QList<QLabel*>{ui->camera_chip_size, ui->camera_pixels_size, ui->camera_bpp, ui->camera_resolution})
          chip_text(widget, "", {-1});
      qDeleteAll(ui->chipInfo->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));

      return;
    }
    cameraDisconnected();
    this->imager = imager.get();
    imager->startLive();
    statusbar_info_widget->deviceConnected(imager->name());
    connect(imager.get(), &Imager::disconnected, q, bind(&Private::cameraDisconnected, this), Qt::QueuedConnection);
    connect(imager.get(), &Imager::fps, statusbar_info_widget, &StatusBarInfoWidget::captureFPS, Qt::QueuedConnection);
    connect(imager.get(), &Imager::temperature, statusbar_info_widget, bind(&StatusBarInfoWidget::temperature, statusbar_info_widget, _1, false), Qt::QueuedConnection);
    ui->camera_name->setText(imager->name());
    auto chip = imager->chip();
    
    chip_text(ui->camera_chip_size, QString("%1x%2").arg(chip.width, 2).arg(chip.height, 2), {chip.width, chip.height});
    chip_text(ui->camera_pixels_size, QString("%1x%2").arg(chip.pixelwidth, 2).arg(chip.pixelheight, 2), {chip.pixelwidth, chip.pixelheight});
    
    chip_text(ui->camera_bpp, "%1"_q % chip.bpp, {chip.bpp});
    chip_text(ui->camera_resolution, "%1x%2"_q % chip.xres % chip.yres, {chip.xres, chip.yres});
    ui->chipInfo->setLayout(new QVBoxLayout);
    for(auto property: chip.properties) {
      qDebug() << "Property name: " << property.name << " = " << property.value;
      ui->chipInfo->layout()->addWidget(new QLabel("%1: %2"_q % property.name % property.value, ui->chipInfo));
    }
    ui->settings_container->setWidget(cameraSettingsWidget = new CameraControlsWidget(imager, settings));
    enableUIWidgets(true);
    ui->actionSelect_ROI->setEnabled(imager->supportsROI());
    ui->actionClear_ROI->setEnabled(imager->supportsROI());
    connect(imager.get(), &Imager::changed, q, [this](const Imager::Control &changed_setting){
      settings_to_save_queue.enqueue(changed_setting);
    }, Qt::QueuedConnection);
    connect(imager.get(), &Imager::fps, q, [this, imager]{
      while(!settings_to_save_queue.isEmpty()) {
        auto setting = settings_to_save_queue.dequeue();
        qDebug() << "Settings changed, camera still alive: " << setting;
        settings.beginGroup(imager->name());
        qDebug() << "setting " << setting.name << " to " << setting.value;
        settings.setValue(setting.name,  setting.value);
        settings.endGroup();
      }
    }, Qt::QueuedConnection);
}


void PlanetaryImagerMainWindow::Private::got_histogram(const vector<uint32_t>& histogram)
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


void PlanetaryImagerMainWindow::Private::cameraDisconnected()
{
  imager = nullptr;
  qDebug() << "camera disconnected";
  enableUIWidgets(false);
    ui->actionSelect_ROI->setEnabled(false);
  ui->actionClear_ROI->setEnabled(false);
  ui->camera_name->clear();
  ui->camera_chip_size->clear();
  ui->camera_bpp->clear();
  ui->camera_pixels_size->clear();
  ui->camera_resolution->clear();
  qDeleteAll(ui->chipInfo->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly));

  delete cameraSettingsWidget;
  cameraSettingsWidget = 0;
  image->setImage({});
  statusbar_info_widget->captureFPS(0);
}

void PlanetaryImagerMainWindow::Private::enableUIWidgets(bool cameraConnected)
{
  ui->actionDisconnect->setEnabled(cameraConnected);
  ui->recording->setEnabled(cameraConnected);
  ui->chipInfoWidget->setEnabled(cameraConnected);
  ui->camera_settings->setEnabled(cameraConnected);
}

#include "planetaryimager_mainwindow.moc"
