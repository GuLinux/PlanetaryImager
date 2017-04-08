/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
 * Copyright (C) 2017  Marco Gulino <marco@gulinux.net>
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

#include "scriptingdialog.h"
#include "ui_scriptingdialog.h"
#include <QPushButton>

using namespace std;

DPTR_IMPL(ScriptingDialog) {
  unique_ptr<Ui::ScriptingDialog> ui;
  ScriptingDialog *q;
};

ScriptingDialog::ScriptingDialog(const ScriptingEngine::ptr &engine, QWidget *parent)
  : QDialog{parent}, dptr(make_unique<Ui::ScriptingDialog>(), this)
{
    d->ui->setupUi(this);
    connect(d->ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::hide);
    connect(d->ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::hide);
    auto runButton = d->ui->buttonBox->addButton(tr("Run"), QDialogButtonBox::ApplyRole);
    connect(runButton, &QPushButton::clicked, this, [=]{ engine->run(d->ui->script->toPlainText()); });
    connect(engine.get(), &ScriptingEngine::reply, [=](const QString &s) { d->ui->output->setText(d->ui->output->toPlainText() + s); });
}

ScriptingDialog::~ScriptingDialog()
{
}
