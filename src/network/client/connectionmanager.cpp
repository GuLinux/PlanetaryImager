/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
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

#include "connectionmanager.h"
#include "ui_connectionmanager.h"

#include "network/client/remotedriver.h"
#include "network/client/networkclient.h"
#include "network/client/remoteconfiguration.h"
#include "network/networkdispatcher.h"
#include "network/client/remotesaveimages.h"

#include "planetaryimager_mainwindow.h"

#include <QPushButton>

#include "Qt/strings.h"
#include "Qt/functional.h"
#include "commons/utils.h"

#include "network/protocol/protocol.h"

using namespace std;

DPTR_IMPL(ConnectionManager) {
  ConnectionManager *q;
  unique_ptr<Ui::ConnectionManager> ui;
  NetworkDispatcher::ptr dispatcher;
  RemoteDriver::ptr remoteDriver;
  NetworkClient::ptr client;
  RemoteConfiguration::ptr configuration;
  
  PlanetaryImagerMainWindow *mainWindow = nullptr;
  
  QHash<int, NetworkProtocol::Format> formats_indexes;
  NetworkProtocol::Format format() const;
  
  void onConnected();
  void adjustParametersVisibility();
};

ConnectionManager::ConnectionManager() : dptr(this)
{
  d->ui = make_unique<Ui::ConnectionManager>();
  d->ui->setupUi(this);
  
  d->formats_indexes = { {0, NetworkProtocol::RAW}, {1, NetworkProtocol::JPEG} };
  
  d->dispatcher = make_shared<NetworkDispatcher>();
  d->client = make_shared<NetworkClient>(d->dispatcher);
  d->remoteDriver = make_shared<RemoteDriver>(d->dispatcher);
  d->configuration = make_shared<RemoteConfiguration>(d->dispatcher);
  auto connectButton = d->ui->buttonBox->addButton(tr("Connect"), QDialogButtonBox::ApplyRole);
  connect(connectButton, &QPushButton::clicked, this, [=] {
    d->configuration->set_server_host(d->ui->host->text());
    d->configuration->set_server_port(d->ui->port->value());
    d->ui->status->setText(tr("Connecting to %1:%2") % d->ui->host->text() % d->ui->port->value());
    d->client->connectToHost(d->ui->host->text(), d->ui->port->value(), d->format(), d->ui->compression->isChecked(), d->ui->force8bit->isChecked(), d->ui->jpeg_quality->value());
  });
  connect(d->ui->host, &QLineEdit::textChanged, this, [=](const QString &newHost) {
    connectButton->setEnabled(! newHost.isEmpty());
  });
  connect(d->client.get(), &NetworkClient::statusChanged, [=](NetworkClient::Status status){
    connectButton->setEnabled(status != NetworkClient::Connecting && status != NetworkClient::Connected);
    if(status == NetworkClient::Connected)
      d->ui->status->setText("Connection established");
    if(status == NetworkClient::Disconnected) {
      if(d->mainWindow) {
        d->mainWindow->close();
        d->mainWindow->deleteLater();
      }
      d->mainWindow = nullptr;
      show();
    }
  });
  
  connect(d->client.get(), &NetworkClient::connected, this, bind(&Private::onConnected, d.get()));
  connect(d->client.get(), &NetworkClient::error, d->ui->status, &QLabel::setText);
  d->ui->host->setText(d->configuration->server_host());
  d->ui->port->setValue(d->configuration->server_port());
  connect(this, &QDialog::finished, qApp, &QApplication::quit);
  connect(d->ui->format, F_PTR(QComboBox, currentIndexChanged, int), this, bind(&Private::adjustParametersVisibility, d.get()));
  
  // TODO: values from configuration
  d->ui->format->setCurrentIndex(0);
  d->ui->compression->setChecked(false);
  d->ui->force8bit->setChecked(false);
  d->ui->jpeg_quality->setValue(85);
  
  d->adjustParametersVisibility();
}

ConnectionManager::~ConnectionManager()
{
  LOG_F_SCOPE
}

void ConnectionManager::Private::onConnected()
{
  ui->status->clear();
  mainWindow = new PlanetaryImagerMainWindow{remoteDriver, make_shared<RemoteSaveImages>(dispatcher), configuration};
  mainWindow->show();
  q->hide();
  if(auto running_camera = remoteDriver->existing_running_camera()) {
    mainWindow->connectCamera(running_camera);
  }
  connect(mainWindow, &PlanetaryImagerMainWindow::quit, client.get(), &NetworkClient::disconnectFromHost);
}

NetworkProtocol::Format ConnectionManager::Private::format() const
{
  return formats_indexes[ui->format->currentIndex()];
}


void ConnectionManager::Private::adjustParametersVisibility()
{
  auto format = this->format();
  ui->compression->setVisible(format == NetworkProtocol::RAW);
  ui->force8bit->setVisible(format == NetworkProtocol::RAW);
  ui->jpeg_quality->setVisible(format == NetworkProtocol::JPEG);
  ui->jpeg_quality_label->setVisible(format == NetworkProtocol::JPEG);
}
