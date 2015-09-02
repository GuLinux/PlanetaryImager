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

#include "configurationdialog.h"
#include "configuration.h"
#include "ui_configurationdialog.h"
#include "utils.h"
#include <functional>
#include "Qt/strings.h"
#include "Qt/functional.h"

using namespace std::placeholders;
using namespace std;

class ConfigurationDialog::Private {
public:
  Private(Configuration &configuration, ConfigurationDialog *q);
  Configuration &configuration;
  shared_ptr<Ui::ConfigurationDialog> ui;
private:
  ConfigurationDialog *q;
};

ConfigurationDialog::Private::Private(Configuration& configuration, ConfigurationDialog* q) : configuration{configuration}, ui{make_shared<Ui::ConfigurationDialog>()}, q{q}
{
}


ConfigurationDialog::~ConfigurationDialog()
{
}

ConfigurationDialog::ConfigurationDialog(Configuration& configuration, QWidget* parent) : QDialog(parent), dptr(configuration, this)
{
    d->ui->setupUi(this);
    
    auto edge_settings = [=] {
      d->ui->cannySettingsBox->setVisible(d->configuration.edgeAlgorithm() == Configuration::Canny);
      d->ui->sobelSettingsBox->setVisible(d->configuration.edgeAlgorithm() == Configuration::Sobel);
    };
    edge_settings();
    QButtonGroup *edgeAlgorithm = new QButtonGroup(this);
    edgeAlgorithm->addButton(d->ui->edge_canny);
    edgeAlgorithm->addButton(d->ui->edge_sobel);
    
    d->ui->edge_canny->setChecked(configuration.edgeAlgorithm() == Configuration::Canny);
    d->ui->edge_sobel->setChecked(configuration.edgeAlgorithm() == Configuration::Sobel);
    
    connect(edgeAlgorithm, F_PTR(QButtonGroup, buttonToggled, QAbstractButton*, bool), [=]{
      d->configuration.setEdgeAlgorithm(edgeAlgorithm->checkedButton() == d->ui->edge_canny ? Configuration::Canny : Configuration::Sobel);
      edge_settings();
    });
    
    d->ui->sobelKernel->setCurrentText(QString::number(d->configuration.sobelKernel()));
    connect(d->ui->sobelKernel, &QComboBox::currentTextChanged, [=](const QString &text) { d->configuration.setSobelKernel(static_cast<Configuration::EdgeAlgorithm>(text.toInt())); });
    
    d->ui->cannyThreshold->setValue(d->configuration.cannyLowThreshold());
    d->ui->cannyRatio->setValue(d->configuration.cannyThresholdRatio());
    connect(d->ui->cannyThreshold, F_PTR(QDoubleSpinBox, valueChanged, double), bind(&Configuration::setCannyLowThreshold, &d->configuration, _1));
    connect(d->ui->cannyRatio, F_PTR(QDoubleSpinBox, valueChanged, double), bind(&Configuration::setCannyThresholdRatio, &d->configuration, _1));
    
    d->ui->buffered_file->setChecked(configuration.bufferedOutput());
    d->ui->drop_view_fps_on_save->setChecked(configuration.maxPreviewFPSOnSaving() > 0);
    d->ui->memory_limit->setRange(0, 500*1024*1024);
    connect(d->ui->memory_limit, &QSlider::valueChanged, [=,&configuration](int value) {
      d->ui->memory_limit_label->setText("%1 MB"_q % QString::number(static_cast<double>(value/(1024.*1024)), 'f', 2));
      configuration.setMaxMemoryUsage(value);
    });
    connect(d->ui->buffered_file, &QCheckBox::toggled, bind(&Configuration::setBufferedOutput, &configuration, _1));
    connect(d->ui->drop_view_fps_on_save, &QCheckBox::toggled, [&configuration](bool checked){ configuration.setMaxPreviewFPSOnSaving(checked ? 10 : 0); });
    d->ui->telescope->setText(configuration.telescope());
    d->ui->observer->setText(configuration.observer());
    connect(d->ui->observer, &QLineEdit::textChanged, bind(&Configuration::setObserver, &configuration, _1));
    connect(d->ui->telescope, &QLineEdit::textChanged, bind(&Configuration::setTelescope, &configuration, _1));
    d->ui->memory_limit->setValue(configuration.maxMemoryUsage());
}
