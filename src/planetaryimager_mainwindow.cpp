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
#include <QThread>
#include <QFileDialog>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent>
#include "fps_counter.h"
#include "widgets/cameracontrolswidget.h"
#include "configurationdialog.h"
#include "configuration.h"
#include <QMutex>
#include <QMessageBox>
#include "displayimage.h"
#include "widgets/recordingpanel.h"
#include "histogram.h"
#include "widgets/camerainfowidget.h"
#include "widgets/histogramwidget.h"
#include "Qt/zoomableimage.h"
#include <QGridLayout>
#include <QToolBar>
#include <QWhatsThis>
#include "Qt/strings.h"
#include <Qt/functional.h>
#include "threadimagehandler.h"

using namespace GuLinux;
using namespace std;
using namespace std::placeholders;


Q_DECLARE_METATYPE(cv::Mat)

DPTR_IMPL(PlanetaryImagerMainWindow) {
  PlanetaryImagerMainWindow *q;
  unique_ptr<Ui::PlanetaryImagerMainWindow> ui;
  DriverPtr driver = make_shared<SupportedDrivers>();
  Imager *imager = nullptr;
  void rescan_devices();
  Configuration configuration;
  void saveState();

  StatusBarInfoWidget *statusbar_info_widget;
  shared_ptr<DisplayImage> displayImage;
  QThread displayImageThread;
  QThread imagerThread;
  shared_ptr<SaveImages> saveImages;
  Histogram::ptr histogram;
  CameraControlsWidget* cameraSettingsWidget = nullptr;
  CameraInfoWidget* cameraInfoWidget = nullptr;
  HistogramWidget *histogramWidget = nullptr;
  ConfigurationDialog *configurationDialog;
    
  RecordingPanel* recording_panel;
  
  void connectCamera(const Driver::CameraPtr &camera);
  void cameraDisconnected();
  void enableUIWidgets(bool cameraConnected);
    void init_devices_watcher();
  ZoomableImage *image;
  
  void onImagerInitialized(Imager *imager);
};

class CreateImagerWorker : public QObject {
  Q_OBJECT
public:
  typedef std::function<void(Imager *)> Slot;
  static void create(const Driver::CameraPtr& camera, const ImageHandlerPtr& imageHandler, QThread* thread, QObject *context, Slot on_created);
private slots:
  void exec();
private:
  explicit CreateImagerWorker(const Driver::CameraPtr& camera, const ImageHandlerPtr& imageHandler);
  Driver::CameraPtr camera;
  ImageHandlerPtr imageHandler;
signals:
  void imager(Imager *imager);
};

CreateImagerWorker::CreateImagerWorker(const Driver::CameraPtr& camera, const ImageHandlerPtr &imageHandler)
  : QObject(nullptr), camera{camera}, imageHandler{imageHandler}
{
}

void CreateImagerWorker::create(const Driver::CameraPtr& camera, const ImageHandlerPtr& imageHandler, QThread* thread, QObject *context, Slot on_created)
{
  auto create_imager = new CreateImagerWorker(camera, imageHandler);
  create_imager->moveToThread(thread);
  connect(create_imager, &CreateImagerWorker::imager, context, on_created, Qt::QueuedConnection);
  QMetaObject::invokeMethod(create_imager, "exec", Qt::QueuedConnection);
}

void CreateImagerWorker::exec()
{
  auto imager = camera->imager(imageHandler);
  if(imager)
    emit this->imager(imager);
  deleteLater();
}


PlanetaryImagerMainWindow::~PlanetaryImagerMainWindow()
{
  LOG_F_SCOPE
  if(d->imager)
      d->imager->stopLive();
  d->imagerThread.quit();
  d->imagerThread.wait();
}

void PlanetaryImagerMainWindow::Private::saveState()
{
  configuration.set_dock_status(q->saveState());
}


PlanetaryImagerMainWindow::PlanetaryImagerMainWindow(QWidget* parent, Qt::WindowFlags flags) : dptr(this)
{
    static bool metatypes_registered = false;
    if(!metatypes_registered) {
      metatypes_registered = true;
      qRegisterMetaType<Frame::ptr>("Frame::ptr");
    }
    d->ui.reset(new Ui::PlanetaryImagerMainWindow);
    d->ui->setupUi(this);
    setWindowIcon(QIcon::fromTheme("planetary_imager"));
    d->ui->recording->setWidget(d->recording_panel = new RecordingPanel{d->configuration});
    d->configurationDialog = new ConfigurationDialog(d->configuration, this);
    d->displayImage = make_shared<DisplayImage>(d->configuration);
    d->saveImages = make_shared<SaveImages>(d->configuration);
    d->histogram = make_shared<Histogram>(d->configuration);
    d->ui->histogram->setWidget(d->histogramWidget = new HistogramWidget(d->histogram, d->configuration));
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
    QToolBar *helpToolBar = new QToolBar;
    helpToolBar->addAction(QWhatsThis::createAction());
    addToolBar(helpToolBar);
    d->image->toolbar()->setFloatable(true);
    d->image->toolbar()->setMovable(true);
    
    restoreState(d->configuration.dock_status());
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

    connect(d->configurationDialog, &QDialog::accepted, this, bind(&DisplayImage::read_settings, d->displayImage), Qt::DirectConnection);
    connect(d->configurationDialog, &QDialog::accepted, this, bind(&Histogram::read_settings, d->histogram), Qt::DirectConnection);
    connect(d->recording_panel, &RecordingPanel::start, [=]{d->saveImages->startRecording(d->imager);});
    connect(d->recording_panel, &RecordingPanel::stop, bind(&SaveImages::endRecording, d->saveImages));
    
    connect(d->saveImages.get(), &SaveImages::recording, this, bind(&DisplayImage::setRecording, d->displayImage, true), Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::recording, this, bind(&Histogram::setRecording, d->histogram, true), Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::recording, d->recording_panel, bind(&RecordingPanel::recording, d->recording_panel, true, _1), Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::finished, d->recording_panel, bind(&RecordingPanel::recording, d->recording_panel, false, QString{}), Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::finished, this, [=]{
        QTimer::singleShot(5000,  bind(&DisplayImage::setRecording, d->displayImage, false));
        QTimer::singleShot(5000,  bind(&Histogram::setRecording, d->histogram, false));
    }, Qt::QueuedConnection);
    setupDockWidget(d->ui->actionChip_Info, d->ui->chipInfoWidget);
    setupDockWidget(d->ui->actionCamera_Settings, d->ui->camera_settings);
    setupDockWidget(d->ui->actionRecording, d->ui->recording);
    setupDockWidget(d->ui->actionHistogram, d->ui->histogram);
    if(! d->configuration.widgets_setup_first_run() ) {
      tabifyDockWidget(d->ui->chipInfoWidget, d->ui->camera_settings);
      tabifyDockWidget(d->ui->chipInfoWidget, d->ui->histogram);
      tabifyDockWidget(d->ui->chipInfoWidget, d->ui->recording);
      d->configuration.set_widgets_setup_first_run(true);
    }
    connect(d->ui->actionHide_all, &QAction::triggered, [=]{ for_each(begin(dock_widgets), end(dock_widgets), bind(&QWidget::hide, _1) ); });
    connect(d->ui->actionShow_all, &QAction::triggered, [=]{ for_each(begin(dock_widgets), end(dock_widgets), bind(&QWidget::show, _1) ); });
    
    d->rescan_devices();
    connect(d->displayImage.get(), &DisplayImage::gotImage, this, bind(&ZoomableImage::setImage, d->image, _1), Qt::QueuedConnection);

    
    connect(d->displayImage.get(), &DisplayImage::displayFPS, d->statusbar_info_widget, &StatusBarInfoWidget::displayFPS, Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::saveFPS, d->recording_panel, &RecordingPanel::saveFPS, Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::meanFPS, d->recording_panel, &RecordingPanel::meanFPS, Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::savedFrames, d->recording_panel, &RecordingPanel::saved, Qt::QueuedConnection);
    connect(d->saveImages.get(), &SaveImages::droppedFrames, d->recording_panel, &RecordingPanel::dropped, Qt::QueuedConnection);
    connect(d->ui->actionDisconnect, &QAction::triggered, this, [=]{
      QMetaObject::invokeMethod(d->imager, "destroy", Qt::QueuedConnection);
    });

    d->enableUIWidgets(false);

    d->saveImages->moveToThread(&d->displayImageThread);
    connect(&d->displayImageThread, &QThread::started, bind(&DisplayImage::create_qimages, d->displayImage));
    d->displayImageThread.start();
    d->imagerThread.start();
    connect(qApp, &QApplication::aboutToQuit, this, [=]{
      if(d->imager)
        d->imager->stopLive();
    }, Qt::QueuedConnection);
    connect(qApp, &QApplication::aboutToQuit, this, [&] {
      d->displayImage->quit();
      d->displayImageThread.quit();
      d->displayImageThread.wait();
      d->imagerThread.quit();
      d->imagerThread.wait();
    });
    connect(d->ui->actionEdges_Detection, &QAction::toggled, [=](bool detect){
      d->displayImage->detectEdges(detect);
    });
    d->init_devices_watcher();
    connect(d->ui->actionClear_ROI, &QAction::triggered, [&] { d->imager->clearROI(); });
    connect(d->ui->actionSelect_ROI, &QAction::triggered, [&] { d->image->startSelectionMode(); });
    connect(d->image, &ZoomableImage::selectedROI, [&](const QRectF &rect) {  // TODO: safety check if we add more selection modes other than ROI
      d->imager->setROI(rect.toRect());
      d->image->clearROI();
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
      QAction *action = ui->menu_device_load->addAction(device->name());
      QObject::connect(action, &QAction::triggered, bind(&Private::connectCamera, this, device));
    }
  });
}

void PlanetaryImagerMainWindow::Private::connectCamera(const Driver::CameraPtr& camera)
{
  auto compositeImageHandler = ImageHandlerPtr{new ImageHandlers{displayImage, saveImages, histogram}};
  auto threadImageHandler = ImageHandlerPtr{new ThreadImageHandler{compositeImageHandler}};
  CreateImagerWorker::create(camera, threadImageHandler, &imagerThread, q, bind(&Private::onImagerInitialized, this, _1) );
}

void PlanetaryImagerMainWindow::Private::onImagerInitialized(Imager * imager)
{  
    if(!imager) {
      return;
    }
    cameraDisconnected();
    this->imager = imager;
    imager->startLive();
    statusbar_info_widget->deviceConnected(imager->name());
    connect(imager, &Imager::disconnected, q, bind(&Private::cameraDisconnected, this), Qt::QueuedConnection);
    connect(imager, &Imager::fps, statusbar_info_widget, &StatusBarInfoWidget::captureFPS, Qt::QueuedConnection);
    connect(imager, &Imager::temperature, statusbar_info_widget, bind(&StatusBarInfoWidget::temperature, statusbar_info_widget, _1, false), Qt::QueuedConnection);

    ui->settings_container->setWidget(cameraSettingsWidget = new CameraControlsWidget(imager, configuration));
    ui->chipInfoWidget->setWidget(cameraInfoWidget = new CameraInfoWidget(imager));
    enableUIWidgets(true);
    ui->actionSelect_ROI->setEnabled(imager->supportsROI());
    ui->actionClear_ROI->setEnabled(imager->supportsROI());
}


void PlanetaryImagerMainWindow::Private::cameraDisconnected()
{
  imager = nullptr;
  qDebug() << "camera disconnected";
  enableUIWidgets(false);
    ui->actionSelect_ROI->setEnabled(false);
  ui->actionClear_ROI->setEnabled(false);
  
  delete cameraSettingsWidget;
  cameraSettingsWidget = nullptr;
  delete cameraInfoWidget;
  cameraInfoWidget = nullptr;
  image->setImage({});
  statusbar_info_widget->captureFPS(0);
  statusbar_info_widget->temperature(0, true);
}

void PlanetaryImagerMainWindow::Private::enableUIWidgets(bool cameraConnected)
{
  ui->actionDisconnect->setEnabled(cameraConnected);
  ui->recording->setEnabled(cameraConnected);
  ui->chipInfoWidget->setEnabled(cameraConnected);
  ui->camera_settings->setEnabled(cameraConnected);
}

#include "planetaryimager_mainwindow.moc"
#include "threadimagehandler.h"
