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

#include "Qt/qt_strings_helper.h"
#include "Qt/qt_functional.h"
#include "commons/utils.h"

#include "network/protocol/protocol.h"
#include "network/client/gui/remotefilesystembrowser.h"
#include "commons/configuration.h"
#include "planetaryimager.h"

using namespace std;

DPTR_IMPL(ConnectionManager) {
  ConnectionManager *q;
  unique_ptr<Ui::ConnectionManager> ui;
  NetworkDispatcherPtr dispatcher;
  RemoteDriverPtr remoteDriver;
  NetworkClientPtr client;
  unique_ptr<RemoteConfiguration> configuration;
  
  PlanetaryImagerMainWindow *mainWindow = nullptr;
  
  QHash<int, Configuration::NetworkImageFormat> formats_indexes;
  Configuration::NetworkImageFormat format() const;
  
  void onConnected();
  void adjustParametersVisibility();
};

ConnectionManager::ConnectionManager() : dptr(this)
{
  d->ui = make_unique<Ui::ConnectionManager>();
  d->ui->setupUi(this);
  
  d->formats_indexes = { {0, Configuration::Network_RAW}, {1, Configuration::Network_JPEG} };
  
  d->dispatcher = make_shared<NetworkDispatcher>();
  d->client = make_shared<NetworkClient>(d->dispatcher);
  d->remoteDriver = make_shared<RemoteDriver>(d->dispatcher);
  d->configuration = make_unique<RemoteConfiguration>(d->dispatcher);
  auto connectButton = d->ui->buttonBox->addButton(tr("Connect"), QDialogButtonBox::ApplyRole);
  connect(connectButton, &QPushButton::clicked, this, [=] {
    d->configuration->set_server_host(d->ui->host->text());
    d->configuration->set_server_port(d->ui->port->value());
    d->ui->status->setText(tr("Connecting to %1:%2") % d->ui->host->text() % d->ui->port->value());
    NetworkProtocol::FormatParameters parameters{d->format(), d->ui->compression->isChecked(), d->ui->force8bit->isChecked(), d->ui->jpeg_quality->value()};
    d->configuration->set_server_image_format(parameters.format);
    d->configuration->set_server_compression(parameters.compression);
    d->configuration->set_server_force8bit(parameters.force8bit);
    d->configuration->set_server_jpeg_quality(parameters.jpegQuality);
    d->client->connectToHost(d->ui->host->text(), d->ui->port->value(), parameters);
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
  d->ui->format->setCurrentIndex(d->formats_indexes.key(d->configuration->server_image_format()));
  d->ui->compression->setChecked(d->configuration->server_compression());
  d->ui->force8bit->setChecked(d->configuration->server_force8bit());
  d->ui->jpeg_quality->setValue(d->configuration->server_jpeg_quality());
  
  d->adjustParametersVisibility();
}

ConnectionManager::~ConnectionManager()
{
  LOG_F_SCOPE
}

// TODO: change with remote
#include "widgets/localfilesystembrowser.h"
void ConnectionManager::Private::onConnected()
{
  ui->status->clear();
  auto imageHandlers = make_shared<ImageHandlers>();
  auto planetaryImager = make_shared<PlanetaryImager>(remoteDriver, imageHandlers, make_shared<RemoteSaveImages>(dispatcher), *configuration);
  mainWindow = new PlanetaryImagerMainWindow{planetaryImager, imageHandlers, make_shared<RemoteFilesystemBrowser>(dispatcher)};
  mainWindow->show();
  q->hide();
  if(auto running_camera = remoteDriver->existing_running_camera()) {
    mainWindow->connectCamera(running_camera);
  }
  connect(mainWindow, &PlanetaryImagerMainWindow::quit, client.get(), &NetworkClient::disconnectFromHost);
}

Configuration::NetworkImageFormat ConnectionManager::Private::format() const
{
  return formats_indexes[ui->format->currentIndex()];
}


void ConnectionManager::Private::adjustParametersVisibility()
{
  auto format = this->format();
  ui->compression->setEnabled(format == Configuration::Network_RAW);
  ui->force8bit->setEnabled(format == Configuration::Network_RAW);
  ui->jpeg_quality->setEnabled(format == Configuration::Network_JPEG);
  ui->jpeg_quality_label->setEnabled(format == Configuration::Network_JPEG);
}
