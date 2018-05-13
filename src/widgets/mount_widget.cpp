/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
 * Copyright (C) 2018  Filip Szczerek <ga.software@yahoo.com>
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

#include <mount_widget.h>
#include <functional>
#include <memory>
#include <QDebug>

#include "mount_dialog.h"
#include "ui_mountwidget.h"


DPTR_IMPL(MountWidget)
{
    std::unique_ptr<Ui::MountWidget> ui;

    MountDialog *mountDialog;
};

MountWidget::~MountWidget()
{
}

MountWidget::MountWidget(QWidget *parent): QWidget(parent), dptr()
{
    d->ui.reset(new Ui::MountWidget);
    d->ui->setupUi(this);

    //d->mountDialog->setLayout(d->ui->verticalLayout);

    d->mountDialog = new MountDialog(this);
    connect(d->ui->btnConnect, &QPushButton::clicked, this, std::bind(&QDialog::open, d->mountDialog));
}
