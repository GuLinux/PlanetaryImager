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

#include "statusbarinfowidget.h"
#include "ui_statusbarinfowidget.h"
#include <QDebug>
#include <QTimer>

using namespace std;

DPTR_IMPL(StatusBarInfoWidget) {
  StatusBarInfoWidget *q;
  unique_ptr<Ui::StatusBarInfoWidget> ui;
  QTimer clear_message_timer;
};

StatusBarInfoWidget::~StatusBarInfoWidget()
{

}

StatusBarInfoWidget::StatusBarInfoWidget(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f), dptr(this)
{
    d->ui = make_unique<Ui::StatusBarInfoWidget>();
    d->ui->setupUi(this);
    captureFPS(0);
    displayFPS(0);
    temperature(0);
    connect(&d->clear_message_timer, &QTimer::timeout, this, &StatusBarInfoWidget::clearMessage);
    d->clear_message_timer.setSingleShot(true);
}

void StatusBarInfoWidget::captureFPS(double fps)
{
  d->ui->capture_fps_frame->setHidden(fps <= 0);
  d->ui->capture_fps->setText(QString::number(fps, 'f', 2));
}

void StatusBarInfoWidget::displayFPS(double fps)
{
  d->ui->display_fps_frame->setHidden(fps <= 0);
  d->ui->display_fps->setText(QString::number(fps, 'f', 2));
}

void StatusBarInfoWidget::temperature(double celsius)
{
  d->ui->temperature_frame->setHidden(celsius <= 0);
  d->ui->temperature->setText(QString::number(celsius, 'f', 2));
}

void StatusBarInfoWidget::showMessage(const QString& message, long int timeout_ms)
{
  d->clear_message_timer.stop();
  d->ui->message->setText(message);
  if(timeout_ms > 0)
    d->clear_message_timer.start(timeout_ms);
}


void StatusBarInfoWidget::clearMessage()
{
  d->ui->message->clear();
}



void StatusBarInfoWidget::deviceConnected(const QString& name)
{
  d->ui->device_status->setText(tr("Connected to %1").arg(name));
}

void StatusBarInfoWidget::deviceDisconnected()
{
  d->ui->device_status->setText(tr("Disconnected"));
}


#include "statusbarinfowidget.moc"