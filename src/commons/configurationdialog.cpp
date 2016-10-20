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

#include "configurationdialog.h"
#include "commons/configuration.h"
#include "ui_configurationdialog.h"
#include "commons/utils.h"
#include <functional>
#include <QSpinBox>
#include <QMessageBox>
#include "Qt/strings.h"
#include "Qt/functional.h"

using namespace std::placeholders;
using namespace std;

DPTR_IMPL(ConfigurationDialog) {
  Configuration &configuration;
  ConfigurationDialog *q;
  unique_ptr<Ui::ConfigurationDialog> ui;
};

ConfigurationDialog::~ConfigurationDialog()
{
}

ConfigurationDialog::ConfigurationDialog(Configuration& configuration, QWidget* parent) : QDialog(parent), dptr(configuration, this)
{
    static bool original_opengl_setting = configuration.opengl();
    d->ui.reset(new Ui::ConfigurationDialog);
    d->ui->setupUi(this);
#ifndef HAVE_QT5_OPENGL
    d->ui->opengl->hide();
#endif
    connect(d->ui->debayer, &QCheckBox::toggled, bind(&Configuration::set_debayer, &configuration, _1));
    connect(d->ui->opengl, &QCheckBox::toggled, bind(&Configuration::set_opengl, &configuration, _1));
    d->ui->debayer->setChecked(configuration.debayer());
    d->ui->opengl->setChecked(configuration.opengl());
    
    auto edge_settings = [=] {
      d->ui->cannySettingsBox->setVisible(d->configuration.edge_algorithm() == Configuration::Canny);
      d->ui->sobelSettingsBox->setVisible(d->configuration.edge_algorithm() == Configuration::Sobel);
    };
    edge_settings();
    QButtonGroup *edgeAlgorithm = new QButtonGroup(this);
    edgeAlgorithm->addButton(d->ui->edge_canny);
    edgeAlgorithm->addButton(d->ui->edge_sobel);
    
    d->ui->edge_canny->setChecked(configuration.edge_algorithm() == Configuration::Canny);
    d->ui->edge_sobel->setChecked(configuration.edge_algorithm() == Configuration::Sobel);
    
    QMap<QAbstractButton*, Configuration::EdgeAlgorithm> edgeAlgorithmWidgets {
        {d->ui->edge_canny, Configuration::Canny},
        {d->ui->edge_sobel, Configuration::Sobel},
    };
    connect(edgeAlgorithm, F_PTR(QButtonGroup, buttonToggled, QAbstractButton*, bool), [=]{
      d->configuration.set_edge_algorithm(edgeAlgorithmWidgets[edgeAlgorithm->checkedButton()]);
      edge_settings();
    });
    
    auto reload_advanced_edge_settings = [=] {
        d->ui->sobelKernel->setCurrentText(QString::number(d->configuration.sobel_kernel()));
        d->ui->sobelScale->setValue(d->configuration.sobel_scale());
        d->ui->sobelDelta->setValue(d->configuration.sobel_delta());
        d->ui->sobelBlurSize->setValue(d->configuration.sobel_blur_size());
        
        d->ui->cannyKernelSize->setValue(d->configuration.canny_kernel_size());
        d->ui->cannyBlurSize->setValue(d->configuration.canny_blur_size());
        d->ui->cannyThreshold->setValue(d->configuration.canny_low_threshold());
        d->ui->cannyRatio->setValue(d->configuration.canny_threshold_ratio());
    };
    reload_advanced_edge_settings();
    connect(d->ui->sobelKernel, &QComboBox::currentTextChanged, [=](const QString &text) { d->configuration.set_sobel_kernel(static_cast<Configuration::EdgeAlgorithm>(text.toInt())); });
    connect(d->ui->sobelBlurSize, F_PTR(QSpinBox, valueChanged, int), [=](int size) { d->configuration.set_sobel_blur_size(size); });
    connect(d->ui->sobelScale, F_PTR(QDoubleSpinBox, valueChanged, double), [=](double scale) { d->configuration.set_sobel_scale(scale); });
    connect(d->ui->sobelDelta, F_PTR(QDoubleSpinBox, valueChanged, double), [=](double delta) { d->configuration.set_sobel_delta(delta); });
    
    connect(d->ui->cannyKernelSize, F_PTR(QSpinBox, valueChanged, int), [=](double size) { d->configuration.set_canny_kernel_size(size); });
    connect(d->ui->cannyBlurSize, F_PTR(QSpinBox, valueChanged, int), [=](double size) { d->configuration.set_canny_blur_size(size); });
    
    connect(d->ui->cannyThreshold, F_PTR(QDoubleSpinBox, valueChanged, double), bind(&Configuration::set_canny_low_threshold, &d->configuration, _1));
    connect(d->ui->cannyRatio, F_PTR(QDoubleSpinBox, valueChanged, double), bind(&Configuration::set_canny_threshold_ratio, &d->configuration, _1));
    
    connect(d->ui->cannyResetDefaults, &QPushButton::clicked, [=]{
        d->configuration.resetCannyAdvancedSettings();
        reload_advanced_edge_settings();
    });
    connect(d->ui->sobelResetDefaults, &QPushButton::clicked, [=]{
        d->configuration.resetSobelAdvancedSettings();
        reload_advanced_edge_settings();
    });
    
    d->ui->buffered_file->setChecked(configuration.buffered_output());
    
    // FPS Limit - not recording
    d->ui->enable_fps_limit->setChecked(configuration.limit_fps());
    d->ui->fps_limit->setValue(configuration.max_display_fps());
    d->ui->fps_limit->setEnabled(configuration.limit_fps());
    connect(d->ui->enable_fps_limit, &QCheckBox::toggled, [this, &configuration] (bool checked){
      configuration.set_limit_fps(checked);
      d->ui->fps_limit->setEnabled(checked);
    });
    connect(d->ui->fps_limit, F_PTR(QSpinBox, valueChanged, int), [ &configuration] (int v){ configuration.set_max_display_fps(v); });
    
    // FPS Limit - recording
    d->ui->enable_fps_limit_recording->setChecked(configuration.limit_fps_recording());
    d->ui->fps_limit_recording->setValue(configuration.max_display_fps_recording());
    d->ui->fps_limit_recording->setEnabled(configuration.limit_fps_recording());
    connect(d->ui->enable_fps_limit_recording, &QCheckBox::toggled, [this, &configuration] (bool checked){
      configuration.set_limit_fps_recording(checked);
      d->ui->fps_limit_recording->setEnabled(checked);
    });
    connect(d->ui->fps_limit_recording, F_PTR(QSpinBox, valueChanged, int), [ &configuration] (int v){ configuration.set_max_display_fps_recording(v); });

    d->ui->histogram_timeout->setValue(configuration.histogram_timeout() / 1000.);
    connect(d->ui->histogram_timeout, F_PTR(QDoubleSpinBox, valueChanged, double), [&configuration](double v) { configuration.set_histogram_timeout(v*1000); });
    d->ui->histogram_disable_on_recording->setChecked(configuration.histogram_disable_on_recording());
    connect(d->ui->histogram_disable_on_recording, &QCheckBox::toggled, [this, &configuration](bool c) {
      d->ui->histogram_timeout_recording->setEnabled(!c);
      configuration.set_histogram_disable_on_recording(c);
    });
    d->ui->histogram_timeout_recording->setEnabled(! configuration.histogram_disable_on_recording());
    d->ui->histogram_timeout_recording->setValue(configuration.histogram_timeout_recording() / 1000.);
    connect(d->ui->histogram_timeout_recording, F_PTR(QDoubleSpinBox, valueChanged, double), [&configuration](double v) { configuration.set_histogram_timeout_recording(v*1000); });
    
    auto set_memory_limit = [=,&configuration](int value) {
      if(value < 1024)
	d->ui->memory_limit_label->setText("%1 MB"_q % value);
      else
	d->ui->memory_limit_label->setText("%1 GB"_q.arg(static_cast<double>(value) / 1024., 0, 'f', 2) );
      configuration.set_max_memory_usage(static_cast<long>(value) * 1024l * 1024l);
    };
    d->ui->memory_limit->setRange(0, 8*1024);
    connect(d->ui->memory_limit, &QSlider::valueChanged, set_memory_limit);
    connect(d->ui->buffered_file, &QCheckBox::toggled, bind(&Configuration::set_buffered_output, &configuration, _1));
    d->ui->memory_limit->setValue(configuration.max_memory_usage() / 1024 / 1024 );
    set_memory_limit(d->ui->memory_limit->value());
        
    d->ui->telescope->setText(configuration.telescope());
    d->ui->observer->setText(configuration.observer());
    connect(d->ui->observer, &QLineEdit::textChanged, bind(&Configuration::set_observer, &configuration, _1));
    connect(d->ui->telescope, &QLineEdit::textChanged, bind(&Configuration::set_telescope, &configuration, _1));
    for(auto codec: QList<QPair<QString,QString>>{
        {"X264", tr("Good compression and quality")},
        {"MJPG", "Motion JPEG, good compression"},
        {"HFYU", "Huffman Lossless Codec"},
        {"ZLIB", "Lossless Codec"},
        {"LZO1", "Lossless Codec"},
        {"ASLC", "Alparysoft Lossless Codec"},
        {"FFV1", "FFMPEG Lossless Codec"},
        {"DIVX", "Old, deprecated"},
        {"XVID", "Old, deprecated"},
    }) {
        d->ui->video_codec->addItem("%1 (%2)"_q % codec.first % codec.second, codec.first);
    }
    d->ui->video_codec->setCurrentIndex(d->ui->video_codec->findData(d->configuration.video_codec()));
    connect(d->ui->video_codec, F_PTR(QComboBox, activated, int), [=](int index){ d->configuration.set_video_codec(d->ui->video_codec->itemData(index).toString()); });
    connect(this, &QDialog::accepted, [&] {
      if(original_opengl_setting != configuration.opengl()) {
        QMessageBox::information(this, tr("Restart required"), tr("You changed the OpenGL setting. Please restart PlanetaryImage for this change to take effect"));
      }
    });
}
