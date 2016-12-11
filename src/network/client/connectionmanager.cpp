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

#include "commons/utils.h"
using namespace std;

DPTR_IMPL(ConnectionManager) {
  ConnectionManager *q;
  unique_ptr<Ui::ConnectionManager> ui;
  NetworkDispatcher::ptr dispatcher;
  RemoteDriver::ptr remoteDriver;
  NetworkClient::ptr client;
  RemoteConfiguration::ptr configuration;
};

ConnectionManager::ConnectionManager() : dptr(this)
{
  d->dispatcher = make_shared<NetworkDispatcher>();
  d->client = make_shared<NetworkClient>(d->dispatcher);
  d->remoteDriver = make_shared<RemoteDriver>(d->dispatcher);
    d->configuration = make_shared<RemoteConfiguration>(d->dispatcher);
    d->ui = make_unique<Ui::ConnectionManager>();
    d->ui->setupUi(this);
    QObject::connect(d->client.get(), &NetworkClient::connected, this, [=]{
      auto mainWindow = new PlanetaryImagerMainWindow{d->remoteDriver, make_shared<RemoteSaveImages>(d->dispatcher), d->configuration};
      mainWindow->show();
      this->hide();
      if(auto running_camera = d->remoteDriver->existing_running_camera()) {
        mainWindow->connectCamera(running_camera);
      }
    });
    auto connectButton = d->ui->buttonBox->addButton(tr("Connect"), QDialogButtonBox::ApplyRole);
    connect(connectButton, &QPushButton::clicked, this, [=] {
      d->configuration->set_server_host(d->ui->host->text());
      d->configuration->set_server_port(d->ui->port->value());
      d->client->connectToHost(d->ui->host->text(), d->ui->port->value());
    });
    d->ui->host->setText(d->configuration->server_host());
    d->ui->port->setValue(d->configuration->server_port());
}

ConnectionManager::~ConnectionManager()
{
  LOG_F_SCOPE
}
