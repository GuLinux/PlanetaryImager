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
using namespace std::placeholders;
using namespace std;

ConfigurationDialog::~ConfigurationDialog()
{
    delete ui;
}

ConfigurationDialog::ConfigurationDialog(Configuration& configuration, QWidget* parent) : QDialog(parent), configuration(configuration)
{
    ui = new Ui::ConfigurationDialog;
    ui->setupUi(this);
    ui->buffered_file->setChecked(configuration.bufferedOutput());
    ui->drop_view_fps_on_save->setChecked(configuration.maxPreviewFPSOnSaving() > 0);
    ui->memory_limit->setRange(0, 500*1024*1024);
    connect(ui->memory_limit, &QSlider::valueChanged, [=,&configuration](int value) {
      ui->memory_limit_label->setText("%1 MB"_q % QString::number(static_cast<double>(value/(1024.*1024)), 'f', 2));
      configuration.setMaxMemoryUsage(value);
    });
    connect(ui->buffered_file, &QCheckBox::toggled, bind(&Configuration::setBufferedOutput, &configuration, _1));
    connect(ui->drop_view_fps_on_save, &QCheckBox::toggled, [&configuration](bool checked){ configuration.setMaxPreviewFPSOnSaving(checked ? 10 : 0); });
    connect(ui->observer, &QLineEdit::textChanged, bind(&Configuration::setObserver, &configuration, _1));
    connect(ui->telescope, &QLineEdit::textChanged, bind(&Configuration::setTelescope, &configuration, _1));
    ui->memory_limit->setValue(configuration.maxMemoryUsage());
}
