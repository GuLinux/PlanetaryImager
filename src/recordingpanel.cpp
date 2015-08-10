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

#include "recordingpanel.h"
#include "ui_recordingpanel.h"
#include "configuration.h"
#include <QFileDialog>

using namespace std;

class RecordingPanel::Private {
public:
  Private(RecordingPanel *q);
    unique_ptr<Ui::RecordingPanel> ui;
    bool recording;
private:
  RecordingPanel *q;
};

RecordingPanel::Private::Private(RecordingPanel* q)
  : q{q}
{

}


RecordingPanel::~RecordingPanel()
{
}

RecordingPanel::RecordingPanel(Configuration& configuration, QWidget* parent) : QWidget{parent}, dpointer(this)
{
  d->ui.reset(new Ui::RecordingPanel);
  d->ui->setupUi(this);
  recording(false);
  d->ui->saveDirectory->setText(configuration.saveDirectory());
  d->ui->saveFramesLimit->setCurrentText(configuration.recordingFramesLimit() == 0 ? tr("Infinite") : QString::number(configuration.recordingFramesLimit()));
  d->ui->filePrefix->setText(configuration.saveFilePrefix());
  d->ui->fileSuffix->setText(configuration.saveFileSuffix());
  connect(d->ui->saveDirectory, &QLineEdit::textChanged, [&configuration](const QString &directory){
    configuration.setSaveDirectory(directory);
  });
  connect(d->ui->filePrefix, &QLineEdit::textChanged, [&configuration](const QString &prefix){
    configuration.setSaveFilePrefix(prefix);
  });
  connect(d->ui->fileSuffix, &QLineEdit::textChanged, [&configuration](const QString &suffix){
    configuration.setSaveFileSuffix(suffix);
  });
  connect(d->ui->saveFramesLimit, &QComboBox::currentTextChanged, [&configuration](const QString &text){
    bool ok = false;
    auto frameLimit = text.toLongLong(&ok);
    if(ok)
      configuration.setRecordingFramesLimit(frameLimit);
    else
      configuration.setRecordingFramesLimit(0);
  });
  connect(d->ui->start_stop_recording, &QPushButton::clicked, [=]{
    if(d->recording)
      emit stop();
    else
      emit start();
  });
  
  auto pickDirectory = d->ui->saveDirectory->addAction(QIcon(":/resources/folder.png"), QLineEdit::TrailingPosition);
  connect(pickDirectory, &QAction::triggered, [&]{
    QFileDialog *filedialog = new QFileDialog(this);
    filedialog->setFileMode(QFileDialog::Directory);
    filedialog->setDirectory(configuration.saveDirectory());
    filedialog->setOption(QFileDialog::ShowDirsOnly);
    connect(filedialog, SIGNAL(fileSelected(QString)), d->ui->saveDirectory, SLOT(setText(QString)));
    connect(filedialog, SIGNAL(finished(int)), filedialog, SLOT(deleteLater()));
    filedialog->show();
  });
  auto check_directory = [&] {
    d->ui->start_stop_recording->setEnabled(QDir(configuration.saveDirectory()).exists());
  };
  check_directory();
  connect(d->ui->saveDirectory, &QLineEdit::textChanged, check_directory);
}

void RecordingPanel::recording(bool recording, const QString& filename)
{
  d->recording = recording;
  saveFPS(0);
  dropped(0);
  d->ui->recordingBox->setVisible(recording);
  d->ui->filename->setText(filename);
  d->ui->start_stop_recording->setText(recording ? tr("Stop") : tr("Start"));
  for(auto widget: QList<QWidget*>{d->ui->saveDirectory, d->ui->filePrefix, d->ui->fileSuffix, d->ui->saveFramesLimit})
    widget->setEnabled(!recording);
}

void RecordingPanel::saveFPS(double fps)
{
  d->ui->fps->setText(QString::number(fps, 'f', 2));
}

void RecordingPanel::dropped(int frames)
{
  d->ui->dropped->setText(QString::number(frames));
}

void RecordingPanel::saved(int frames)
{
  d->ui->frames->setText(QString::number(frames));
}



