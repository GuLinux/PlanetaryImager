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
#include "drivers/imager.h"
#include "ui_planetaryimager_mainwindow.h"
#include <functional>
#include "commons/utils.h"
#include "widgets/statusbarinfowidget.h"
#include <QLabel>
#include <QDoubleSpinBox>
#include <QThread>
#include <QFileDialog>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent>
#include "commons/fps_counter.h"
#include "widgets/cameracontrolswidget.h"
#include "widgets/configurationdialog.h"
#include "commons/configuration.h"
#include <QMutex>
#include <QMessageBox>
#include "widgets/recordingpanel.h"
#include "widgets/camerainfowidget.h"
#include "widgets/histogramwidget.h"
#include "Qt/zoomableimage.h"
#include <QGridLayout>
#include <QToolBar>
#include <QWhatsThis>
#include "Qt/qt_strings_helper.h"
#include "Qt/qt_functional.h"
#include <QGraphicsScene>
#include <QFileInfo>
#include <QDesktopServices>

#include "widgets/editroidialog.h"

#include "image_handlers/frontend/displayimage.h"
#include "image_handlers/saveimages.h"
#include "image_handlers/threadimagehandler.h"

#include "commons/messageslogger.h"
#include "c++/stlutils.h"
#include "commons/exposuretimer.h"
#include "Qt/qt_functional.h"

#include "planetaryimager.h"

using namespace GuLinux;
using namespace std;
using namespace std::placeholders;


Q_DECLARE_METATYPE(cv::Mat)

DPTR_IMPL(PlanetaryImagerMainWindow) {
  PlanetaryImager::ptr planetaryImager;
  FilesystemBrowser::ptr filesystemBrowser;
  
  static PlanetaryImagerMainWindow *q;
  unique_ptr<Ui::PlanetaryImagerMainWindow> ui;
  Imager *imager = nullptr;
  void rescan_devices();
  void saveState();

  StatusBarInfoWidget *statusbar_info_widget;
  shared_ptr<DisplayImage> displayImage;
  Histogram::ptr histogram;
  CameraControlsWidget* cameraSettingsWidget = nullptr;
  CameraInfoWidget* cameraInfoWidget = nullptr;
  HistogramWidget *histogramWidget = nullptr;
  ConfigurationDialog *configurationDialog;
  EditROIDialog *editROIDialog;
  
  RecordingPanel* recording_panel;
  ExposureTimer exposure_timer;
  
  ImageHandler::ptr imageHandler;
  
  
  void cameraDisconnected();
  void enableUIWidgets(bool cameraConnected);
  void editROI();
  ZoomableImage *image_widget;
  
  void onImagerInitialized(Imager *imager);
  void onCamerasFound();
  enum SelectionMode { NoSelection, ROI, Guide } selection_mode = NoSelection;
};

PlanetaryImagerMainWindow *PlanetaryImagerMainWindow::Private::q = nullptr;


PlanetaryImagerMainWindow::~PlanetaryImagerMainWindow()
{
  LOG_F_SCOPE
  d->saveState();
  if(d->imager) {
      d->imager->destroy();
  }
  d->displayImage->quit();
  d->planetaryImager->quit();
}

void PlanetaryImagerMainWindow::Private::saveState()
{
  planetaryImager->configuration().set_dock_status(q->saveState());
  planetaryImager->configuration().set_main_window_geometry(q->saveGeometry());
}


PlanetaryImagerMainWindow::PlanetaryImagerMainWindow(
      const PlanetaryImager::ptr &planetaryImager,
      const ImageHandlers::ptr &imageHandlers,
      const FilesystemBrowser::ptr &filesystemBrowser,
      const QString &logFilePath,
      QWidget* parent,
      Qt::WindowFlags flags
  )
: QMainWindow(parent, flags), dptr(planetaryImager, filesystemBrowser)
{
    Private::q = this;
    d->ui.reset(new Ui::PlanetaryImagerMainWindow);
    
    d->ui->setupUi(this);
    setWindowIcon(QIcon::fromTheme("planetary_imager"));
    d->ui->recording->setWidget(d->recording_panel = new RecordingPanel{d->planetaryImager->configuration(), filesystemBrowser});
    d->configurationDialog = new ConfigurationDialog(d->planetaryImager->configuration(), this);
    d->displayImage = make_shared<DisplayImage>(d->planetaryImager->configuration());
    d->histogram = make_shared<Histogram>(d->planetaryImager->configuration());
    d->ui->histogram->setWidget(d->histogramWidget = new HistogramWidget(d->histogram, d->planetaryImager->configuration()));
    d->ui->statusbar->addPermanentWidget(d->statusbar_info_widget = new StatusBarInfoWidget(), 1);

    imageHandlers->push_back(d->displayImage);
    imageHandlers->push_back(d->histogram);
    
    connect(d->planetaryImager.get(), &PlanetaryImager::camerasChanged, this, bind(&Private::onCamerasFound, d.get()));
    connect(d->planetaryImager.get(), &PlanetaryImager::cameraConnected, this, [=]{ d->onImagerInitialized(d->planetaryImager->imager()); });
    d->ui->image->setLayout(new QGridLayout);
    d->ui->image->layout()->setMargin(0);
    d->ui->image->layout()->setSpacing(0);
    d->ui->image->layout()->addWidget(d->image_widget = new ZoomableImage(false));
#ifdef HAVE_QT5_OPENGL // TODO: make configuration item
    if(d->planetaryImager->configuration().opengl())
      d->image_widget->setOpenGL();
#endif
    d->image_widget->scene()->setBackgroundBrush(QBrush{Qt::black, Qt::Dense4Pattern});
    connect(d->image_widget, &ZoomableImage::zoomLevelChanged, d->statusbar_info_widget, &StatusBarInfoWidget::zoom);
    d->statusbar_info_widget->zoom(d->image_widget->zoomLevel());
    for(auto item: d->image_widget->actions())
      d->ui->menuView->insertAction(d->ui->actionEdges_Detection, item);
    d->ui->menuView->insertSeparator(d->ui->actionEdges_Detection);
    
    d->image_widget->actions()[ZoomableImage::Actions::ZoomIn]->setShortcut({Qt::CTRL + Qt::Key_Plus});
    d->image_widget->actions()[ZoomableImage::Actions::ZoomIn]->setIcon(QIcon{":/resources/zoom_in.png"});
    d->image_widget->actions()[ZoomableImage::Actions::ZoomOut]->setShortcut({Qt::CTRL + Qt::Key_Minus});
    d->image_widget->actions()[ZoomableImage::Actions::ZoomOut]->setIcon(QIcon{":/resources/zoom_out.png"});
    d->image_widget->actions()[ZoomableImage::Actions::ZoomFit]->setShortcut({Qt::CTRL + Qt::Key_Space});
    d->image_widget->actions()[ZoomableImage::Actions::ZoomFit]->setIcon(QIcon{":/resources/fit_window.png"});
    d->image_widget->actions()[ZoomableImage::Actions::ZoomRealSize]->setShortcut({Qt::CTRL + Qt::Key_Backspace});
    d->image_widget->actions()[ZoomableImage::Actions::ZoomRealSize]->setIcon(QIcon{":/resources/real_size.png"});
    d->image_widget->toolbar()->setWindowTitle("Image Control");
    addToolBar(d->image_widget->toolbar());
    QToolBar *helpToolBar = new QToolBar;
    helpToolBar->setWindowTitle(tr("Help"));
    auto whatsThis = QWhatsThis::createAction();
    whatsThis->setIcon(QIcon{":/resources/help.png"});
    helpToolBar->addAction(whatsThis);
    addToolBar(helpToolBar);
    d->image_widget->toolbar()->setFloatable(true);
    d->image_widget->toolbar()->setMovable(true);
    
    restoreGeometry(d->planetaryImager->configuration().main_window_geometry());
    restoreState(d->planetaryImager->configuration().dock_status());
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
    connect(d->configurationDialog, &QDialog::accepted, this,
            [this]() {
                auto *imager = d->planetaryImager->imager();
                if (imager) imager->setCaptureEndianess(d->planetaryImager->configuration().capture_endianess());
            });

    connect(d->recording_panel, &RecordingPanel::start, [=]{d->planetaryImager->saveImages()->startRecording(d->imager);});
    connect(d->recording_panel, &RecordingPanel::stop, bind(&SaveImages::endRecording, d->planetaryImager->saveImages()));
    connect(d->recording_panel, &RecordingPanel::setPaused, bind(&SaveImages::setPaused, d->planetaryImager->saveImages(), _1));
    
    connect(d->planetaryImager->saveImages().get(), &SaveImages::recording, this, bind(&DisplayImage::setRecording, d->displayImage, true), Qt::QueuedConnection);
    connect(d->planetaryImager->saveImages().get(), &SaveImages::recording, this, bind(&Histogram::setRecording, d->histogram, true), Qt::QueuedConnection);
    connect(d->planetaryImager->saveImages().get(), &SaveImages::recording, d->recording_panel, bind(&RecordingPanel::recording, d->recording_panel, true, _1), Qt::QueuedConnection);
    connect(d->planetaryImager->saveImages().get(), &SaveImages::finished, d->recording_panel, bind(&RecordingPanel::recording, d->recording_panel, false, QString{}), Qt::QueuedConnection);
    connect(d->planetaryImager->saveImages().get(), &SaveImages::finished, this, [=]{
        QTimer::singleShot(5000,  bind(&DisplayImage::setRecording, d->displayImage, false));
        QTimer::singleShot(5000,  bind(&Histogram::setRecording, d->histogram, false));
    }, Qt::QueuedConnection);
    setupDockWidget(d->ui->actionChip_Info, d->ui->chipInfoWidget);
    setupDockWidget(d->ui->actionCamera_Settings, d->ui->camera_settings);
    setupDockWidget(d->ui->actionRecording, d->ui->recording);
    setupDockWidget(d->ui->actionHistogram, d->ui->histogram);
    if(! d->planetaryImager->configuration().widgets_setup_first_run() ) {
      tabifyDockWidget(d->ui->chipInfoWidget, d->ui->camera_settings);
      tabifyDockWidget(d->ui->chipInfoWidget, d->ui->histogram);
      tabifyDockWidget(d->ui->chipInfoWidget, d->ui->recording);
      d->planetaryImager->configuration().set_widgets_setup_first_run(true);
    }
    qDebug() << "file " << logFilePath << "exists: " << QFile::exists(logFilePath);
    d->ui->actionOpen_log_file_folder->setMenuRole(QAction::ApplicationSpecificRole);
    d->ui->actionOpen_log_file_folder->setVisible( ! logFilePath.isEmpty() && QFile::exists(logFilePath));
    connect(d->ui->actionOpen_log_file_folder, &QAction::triggered, this, [=]{
#ifdef Q_OS_MAC
        QStringList args;
        args << "-e";
        args << "tell application \"Finder\"";
        args << "-e";
        args << "activate";
        args << "-e";
        args << "select POSIX file \""+logFilePath+"\"";
        args << "-e";
        args << "end tell";
        QProcess::startDetached("osascript", args);
#else
          auto logFileDirectory = QFileInfo{logFilePath}.dir().path();
          if(!QDesktopServices::openUrl(logFileDirectory)) {
            MessagesLogger::instance()->queue(MessagesLogger::Warning, tr("Log file"), tr("Unable to open your log file. You can open it manually at this position: %1") % logFileDirectory);
          }
#endif
    });
    
    connect(d->ui->actionHide_all, &QAction::triggered, [=]{ for_each(begin(dock_widgets), end(dock_widgets), bind(&QWidget::hide, _1) ); });
    connect(d->ui->actionShow_all, &QAction::triggered, [=]{ for_each(begin(dock_widgets), end(dock_widgets), bind(&QWidget::show, _1) ); });
    connect(MessagesLogger::instance(), &MessagesLogger::message, this, bind(&PlanetaryImagerMainWindow::notify, this, _1, _2, _3, _4), Qt::QueuedConnection);
    connect(d->displayImage.get(), &DisplayImage::gotImage, this, bind(&ZoomableImage::setImage, d->image_widget, _1), Qt::QueuedConnection);
    
    connect(d->ui->actionNight_Mode, &QAction::toggled, this, [=](bool checked) {
      qApp->setStyleSheet(checked ? R"_(
        * { background-color: rgb(40, 0, 0); color: rgb(220, 220, 220); }
      )_" : "");
    });

    
    connect(d->displayImage.get(), &DisplayImage::displayFPS, d->statusbar_info_widget, &StatusBarInfoWidget::displayFPS, Qt::QueuedConnection);
    connect(d->planetaryImager->saveImages().get(), &SaveImages::saveFPS, d->recording_panel, &RecordingPanel::saveFPS, Qt::QueuedConnection);
    connect(d->planetaryImager->saveImages().get(), &SaveImages::meanFPS, d->recording_panel, &RecordingPanel::meanFPS, Qt::QueuedConnection);
    connect(d->planetaryImager->saveImages().get(), &SaveImages::savedFrames, d->recording_panel, &RecordingPanel::saved, Qt::QueuedConnection);
    connect(d->planetaryImager->saveImages().get(), &SaveImages::droppedFrames, d->recording_panel, &RecordingPanel::dropped, Qt::QueuedConnection);
    connect(d->ui->actionDisconnect, &QAction::triggered, d->planetaryImager.get(), &PlanetaryImager::closeImager);

    connect(d->ui->actionQuit, &QAction::triggered, this, &QWidget::close);
    connect(d->ui->actionQuit, &QAction::triggered, this, &PlanetaryImagerMainWindow::quit);
    d->enableUIWidgets(false);

    QtConcurrent::run(bind(&DisplayImage::create_qimages, d->displayImage));


    connect(d->ui->actionEdges_Detection, &QAction::toggled, d->displayImage.get(), &DisplayImage::detectEdges);
    connect(d->ui->actionStretch_histogram, &QAction::toggled, d->displayImage.get(), &DisplayImage::histogramEqualization);
    connect(d->ui->actionStretch_colour_saturation, &QAction::toggled, d->displayImage.get(), &DisplayImage::maximumSaturation);

    
    connect(d->ui->actionClear_ROI, &QAction::triggered, [&] { d->imager->clearROI(); });
    connect(d->ui->actionSelect_ROI, &QAction::triggered, [&] { 
      d->selection_mode = Private::ROI;
      d->image_widget->startSelectionMode();
    });
    
    connect(d->ui->actionEdit_ROI, &QAction::triggered, this, bind(&Private::editROI, d.get()));
    QMap<Private::SelectionMode, function<void(const QRect &)>> handle_selection {
      {Private::NoSelection, [](const QRect&) {}},
      {Private::ROI, [&](const QRect &rect) { d->imager->setROI(rect.normalized()); }},
    };
    connect(d->image_widget, &ZoomableImage::selectedROI, [this, handle_selection](const QRectF &rect) {
      handle_selection[d->selection_mode](rect.toRect());
      d->image_widget->clearROI();
      d->selection_mode = Private::NoSelection;
    });
    connect(&d->exposure_timer, &ExposureTimer::progress, [=](double , double elapsed, double remaining){
      d->statusbar_info_widget->showMessage("Exposure: %1s, remaining: %2s"_q % QString::number(elapsed, 'f', 1) % QString::number(remaining, 'f', 1), 1000);
    });
    connect(&d->exposure_timer, &ExposureTimer::finished, [=]{ d->statusbar_info_widget->clearMessage(); });
    
    d->editROIDialog = new EditROIDialog(this);
    connect(d->editROIDialog, &EditROIDialog::roiSelected, this, [=](const QRect &roi){ d->imager->setROI(roi); });
    auto readTemperature = new QTimer{this};
    connect(readTemperature, &QTimer::timeout, this, [=] {
        if(d->imager)
            QMetaObject::invokeMethod(d->imager, "readTemperature", Qt::QueuedConnection);
    });
    readTemperature->start(2000);
    d->rescan_devices();
}

void PlanetaryImagerMainWindow::closeEvent(QCloseEvent* event)
{
  QMainWindow::closeEvent(event);
  emit quit();
}

ImageHandler::ptr PlanetaryImagerMainWindow::imageHandler() const
{
  return d->imageHandler;
}


Imager * PlanetaryImagerMainWindow::imager() const
{
  return d->imager;
}


void PlanetaryImagerMainWindow::Private::rescan_devices()
{
  ui->menu_device_load->clear();
  planetaryImager->scanCameras();
}

void PlanetaryImagerMainWindow::Private::onCamerasFound()
{
  ui->menu_device_load->clear();
  auto message = tr("Found %1 devices").arg(planetaryImager->cameras().size());
  for(auto device: planetaryImager->cameras()) {
    qDebug() << message;
    statusbar_info_widget->showMessage(message, 10'000);
    QAction *action = ui->menu_device_load->addAction(device->name());
    QObject::connect(action, &QAction::triggered, bind(&PlanetaryImager::open, planetaryImager.get(), device));
  }
}


void PlanetaryImagerMainWindow::connectCamera(const Driver::Camera::ptr& camera)
{
  d->planetaryImager->open(camera);
}


void PlanetaryImagerMainWindow::setImager(Imager* imager)
{
  d->onImagerInitialized(imager);
}


void PlanetaryImagerMainWindow::Private::onImagerInitialized(Imager * imager)
{  
    GuLinux::Scope scope{[=]{  emit q->imagerChanged(); }};
    this->imager = imager;
    exposure_timer.set_imager(imager);
    imager->startLive();
    statusbar_info_widget->deviceConnected(imager->name());
    connect(imager, &Imager::disconnected, q, bind(&Private::cameraDisconnected, this), Qt::QueuedConnection);
    connect(imager, &Imager::fps, statusbar_info_widget, &StatusBarInfoWidget::captureFPS, Qt::QueuedConnection);
    connect(imager, &Imager::temperature, statusbar_info_widget, bind(&StatusBarInfoWidget::temperature, statusbar_info_widget, _1, false), Qt::QueuedConnection);

    ui->settings_container->setWidget(cameraSettingsWidget = new CameraControlsWidget(imager, planetaryImager->configuration(), filesystemBrowser));
    ui->chipInfoWidget->setWidget(cameraInfoWidget = new CameraInfoWidget(imager));
    enableUIWidgets(true);
    ui->actionSelect_ROI->setEnabled(imager->supports(Imager::ROI));
    ui->actionEdit_ROI->setEnabled(imager->supports(Imager::ROI));
    ui->actionClear_ROI->setEnabled(imager->supports(Imager::ROI));
}

// TODO: sync issues when images are sent after the imagerDisconnected signal
void PlanetaryImagerMainWindow::Private::cameraDisconnected()
{
  imager = nullptr;
  qDebug() << "camera disconnected";
  enableUIWidgets(false);
  ui->actionSelect_ROI->setEnabled(false);
  ui->actionEdit_ROI->setEnabled(false);
  ui->actionClear_ROI->setEnabled(false);
  
  delete cameraSettingsWidget;
  cameraSettingsWidget = nullptr;
  delete cameraInfoWidget;
  cameraInfoWidget = nullptr;
  statusbar_info_widget->captureFPS(0);
  statusbar_info_widget->temperature(0, true);
  image_widget->setImage({});
}

void PlanetaryImagerMainWindow::Private::enableUIWidgets(bool cameraConnected)
{
  ui->actionDisconnect->setEnabled(cameraConnected);
  ui->recording->setEnabled(cameraConnected);
  ui->chipInfoWidget->setEnabled(cameraConnected);
  ui->camera_settings->setEnabled(cameraConnected);
}

void PlanetaryImagerMainWindow::notify(const QDateTime &when, MessagesLogger::Type notification_type, const QString& title, const QString& message)
{
  static QHash<MessagesLogger::Type, function<void(const QString&, const QString&)>> types_map {
    {MessagesLogger::Warning, [](const QString &title, const QString &message) { QMessageBox::warning(nullptr, title, message); }},
    {MessagesLogger::Error, [](const QString &title, const QString &message) { QMessageBox::critical(nullptr, title, message); }},
    {MessagesLogger::Info, [](const QString &title, const QString &message) { QMessageBox::information(nullptr, title, message); }},
  };
  types_map[notification_type](title, message);
}


void PlanetaryImagerMainWindow::Private::editROI()
{
  auto resolution = imager->properties().resolution();
  editROIDialog->setResolution(resolution);
  editROIDialog->show();
  //TODO check for resolution not existing
  // TODO check for current ROI
}



#include "planetaryimager_mainwindow.moc"
