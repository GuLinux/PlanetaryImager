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

#include "recordingpanel.h"
#include "ui_recordingpanel.h"
#include "commons/configuration.h"
#include "savefileconfiguration.h"
#include "commons/messageslogger.h"
#include "Qt/qt_functional.h"
#include <QAction>
#include <QDir>
#include <QDialog>
#include <QAction>
#include <QElapsedTimer>
#include <QTimer>
#include <QTime>
#include <QFileInfo>
#include <QDebug>
#include "commons/elapsedtimer.h"
#include "commons/filesystembrowser.h"
#include "c++/stlutils.h"

using namespace std;

DPTR_IMPL(RecordingPanel) {
    Configuration &configuration;
    FilesystemBrowserPtr filesystemBrowser;
    bool recording;
    RecordingPanel *q;
    unique_ptr<Ui::RecordingPanel> ui;
    ElapsedTimer recording_elapsed;
    unique_ptr<QTimer> recording_elapsed_timer;
    void reload_config();
    static vector<Configuration::SaveFormat> format_combo_index;
    bool is_reloading_prefix_suffix{false};
};

vector<Configuration::SaveFormat> RecordingPanel::Private::format_combo_index = {
  Configuration::SER,
  Configuration::Video,
  Configuration::PNG,
  Configuration::FITS,
};

RecordingPanel::~RecordingPanel()
{
}

RecordingPanel::RecordingPanel(Configuration &configuration, const FilesystemBrowserPtr &filesystemBrowser, QWidget* parent)
  : QWidget{parent}, dptr(configuration, filesystemBrowser, false, this)
{
  d->ui = make_unique<Ui::RecordingPanel>();
  d->ui->setupUi(this);
  d->recording_elapsed_timer = make_unique<QTimer>();
  connect(d->recording_elapsed_timer.get(), &QTimer::timeout, this, [this]{
    d->ui->elapsed->setText(QTime{0,0,0}.addMSecs(d->recording_elapsed.milliseconds()).toString());
  });
  
  recording(false);

  connect(d->ui->videoOutputType, F_PTR(QComboBox, activated, int), [&](int index) {
    configuration.set_save_format(Private::format_combo_index[index]);
    if(Private::format_combo_index[index] == Configuration::Video && !configuration.deprecated_video_warning_shown()) {
      configuration.set_deprecated_video_warning_shown(true);
      MessagesLogger::instance()->queue(MessagesLogger::Warning, tr("Video encoder"), tr(R"(Saving as video file is not supported, and not recommended.
Video encoding might fail, due to unavailable codecs, will result in quality loss, and will slow down capture FPS.
The best file format for planetary imaging is SER. You can also save frames as PNG or TIFF images.)"));
    }
  });
  connect(d->ui->saveDirectory, &QLineEdit::textChanged, [this, &configuration](const QString &directory){
    configuration.set_save_directory(directory);
  });
  
  
  auto change_prefix = [this, &configuration](const QString &prefix){
    if(! d->is_reloading_prefix_suffix) configuration.set_save_file_prefix(prefix);
  };
  auto change_suffix = [this, &configuration](const QString &suffix){
    if(! d->is_reloading_prefix_suffix) configuration.set_save_file_suffix(suffix);
  };
  connect(d->ui->filePrefix, F_PTR(QComboBox, editTextChanged, const QString&), this, change_prefix);
  connect(d->ui->fileSuffix, F_PTR(QComboBox, editTextChanged, const QString&), this, change_suffix);
  
  connect(d->ui->limitType, F_PTR(QComboBox, activated, int), d->ui->limitsWidgets, &QStackedWidget::setCurrentIndex);
  connect(d->ui->limitType, F_PTR(QComboBox, activated, int), [&configuration](int index){
    configuration.set_recording_limit_type(static_cast<Configuration::RecordingLimit>(index));
  });
  d->ui->limitsWidgets->setCurrentIndex(d->ui->limitType->currentIndex());
  
  connect(d->ui->filename_options, &QPushButton::clicked, [&] {
      QDialog *dialog = new QDialog;
      dialog->setWindowTitle(tr("Filename options"));
      dialog->setLayout(new QVBoxLayout);
      auto saveFileConfiguration = new SaveFileConfiguration(configuration, dialog);
      dialog->layout()->addWidget(saveFileConfiguration);
      dialog->resize(550, 100);
      dialog->exec();
      dialog->deleteLater();
  });
  connect(d->ui->saveFramesLimit, &QComboBox::currentTextChanged, [&configuration](const QString &text){
      configuration.set_recording_frames_limit(text.toLongLong());
  });
  
  connect(d->ui->duration_limit, F_PTR(QDoubleSpinBox, valueChanged, double), [&configuration](double seconds){
      configuration.set_recording_seconds_limit(seconds);
  });
  
  connect(d->ui->save_recording_info, F_PTR(QComboBox, activated, int), [&configuration](int index) {
    switch(index) {
      case 0:
        configuration.set_save_info_file(false);
        configuration.set_save_json_info_file(false);
        break;
      case 1:
        configuration.set_save_info_file(false);
        configuration.set_save_json_info_file(true);
        break;
      case 2:
        configuration.set_save_info_file(true);
        configuration.set_save_json_info_file(false);
        break;
      case 3:
        configuration.set_save_info_file(true);
        configuration.set_save_json_info_file(true);
        break;
    }
  });
  connect(d->ui->start_recording, &QPushButton::clicked, this, &RecordingPanel::start);
  connect(d->ui->stop_recording, &QPushButton::clicked, this, &RecordingPanel::stop);
  connect(d->ui->pause_recording, &QPushButton::toggled, this, &RecordingPanel::setPaused);
  connect(this, &RecordingPanel::setPaused, this, [=, &configuration](bool paused) {
    d->ui->pause_recording->setIcon(QIcon{paused ? ":/resources/play.png" : ":/resources/pause.png"});
    if(configuration.recording_pause_stops_timer()) {
      if(paused)
        d->recording_elapsed.pause();
      else
        d->recording_elapsed.resume();
    }
  });
  
  auto pickDirectory = d->ui->saveDirectory->addAction(QIcon(":/resources/folder.png"), QLineEdit::TrailingPosition);
  connect(pickDirectory, &QAction::triggered, d->filesystemBrowser.get(), [=, &configuration]{ d->filesystemBrowser->pickDirectory(configuration.save_directory()); });
  connect(d->filesystemBrowser.get(), &FilesystemBrowser::directoryPicked, d->ui->saveDirectory, &QLineEdit::setText);

  auto check_directory = [&] {
    auto saveDir = QDir(configuration.save_directory());
    d->ui->start_recording->setEnabled(
      !configuration.save_directory().isEmpty() &&
      saveDir.exists() && 
      QFileInfo(saveDir.path()).isWritable()
    );
  };
  check_directory();
  connect(d->ui->saveDirectory, &QLineEdit::textChanged, check_directory);
  connect(d->ui->timelapse, &QCheckBox::toggled, this, [=, &configuration](bool checked) {
    d->ui->timelapse_frame->setVisible(checked);
    configuration.set_timelapse_mode(checked);
  });
  connect(d->ui->timelapse_duration, &QTimeEdit::timeChanged, this, [=, &configuration](const QTime &time) {
    configuration.set_timelapse_msecs(QTime{0,0,0}.msecsTo(time));
  });

  d->reload_config();
  connect(&configuration, &Configuration::settings_changed, this, bind(&Private::reload_config, d.get()));
}


void RecordingPanel::Private::reload_config()
{
  ui->save_recording_info->setCurrentIndex( (configuration.save_json_info_file() ? 1 : 0) + (configuration.save_info_file() ? 2 : 0) );
  
  ui->timelapse->setChecked(configuration.timelapse_mode());
  ui->timelapse_duration->setTime(QTime{0,0,0}.addMSecs(configuration.timelapse_msecs()));
  ui->saveDirectory->setText(configuration.save_directory());
  ui->duration_limit->setValue(configuration.recording_seconds_limit());
  ui->saveFramesLimit->setCurrentText(QString::number(configuration.recording_frames_limit()));
  ui->limitType->setCurrentIndex(configuration.recording_limit_type());
  auto current_format_index = std::find(format_combo_index.begin(), format_combo_index.end(), configuration.save_format());
  ui->videoOutputType->setCurrentIndex( current_format_index == format_combo_index.end() ? 0 : current_format_index-format_combo_index.begin() );
  is_reloading_prefix_suffix = true;
  GuLinux::Scope reset_reloading([&]{ is_reloading_prefix_suffix = false; });
  ui->filePrefix->clear();
  ui->filePrefix->addItems(configuration.save_file_avail_prefixes());
  ui->filePrefix->setCurrentText(configuration.save_file_prefix());
  ui->fileSuffix->clear();
  ui->fileSuffix->addItems(configuration.save_file_avail_suffixes());
  ui->fileSuffix->setCurrentText(configuration.save_file_suffix());
  
  ui->filePrefix->setCurrentText(configuration.save_file_prefix());
  ui->fileSuffix->setCurrentText(configuration.save_file_suffix());
}


void RecordingPanel::recording(bool recording, const QString& filename)
{
  d->recording = recording;
  saveFPS(0);
  dropped(0);
  d->ui->recordingBox->setVisible(recording);
  d->ui->filename->setText(filename);
  d->ui->recordingButtons->setCurrentIndex(recording ? 1 : 0);
  for(auto widget: QList<QWidget*>{d->ui->saveDirectory, d->ui->filePrefix, d->ui->fileSuffix, d->ui->saveFramesLimit})
    widget->setEnabled(!recording);
  if(recording) {
    d->recording_elapsed.start();
    d->recording_elapsed_timer->start(500);
  } else {
    d->recording_elapsed_timer->stop();
  }
}

void RecordingPanel::saveFPS(double fps)
{
  d->ui->fps->setText(QString::number(fps, 'f', 2));
}

void RecordingPanel::meanFPS(double fps)
{
  d->ui->mean_fps->setText(QString::number(fps, 'f', 2));
}


void RecordingPanel::dropped(long frames)
{
  d->ui->dropped->setText(QString::number(frames));
}

void RecordingPanel::saved(long frames)
{
  d->ui->frames->setText(QString::number(frames));
}



