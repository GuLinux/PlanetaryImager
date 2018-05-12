/*
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

#include <memory>

#include "mount_dialog.h"
#include "ui_mount_dialog.h"


DPTR_IMPL(MountDialog) {

  MountDialog *q;
  std::unique_ptr<Ui::MountDialog> ui;
};

MountDialog::~MountDialog()
{
}

MountDialog::MountDialog(QWidget *parent): QDialog(parent), dptr(this)
{
    d->ui.reset(new Ui::MountDialog);
    d->ui->setupUi(this);
}
