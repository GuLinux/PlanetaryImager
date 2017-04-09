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

#include "networkserver.h"
#include <QtNetwork/QTcpServer>
#include "network/server/driverforwarder.h"
#include "network/networkdispatcher.h"
#include "network/protocol/protocol.h"
#include "network/protocol/driverprotocol.h"
#include "Qt/strings.h"
#include <QElapsedTimer>
#include "network/server/filesystemforwarder.h"

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(NetworkServer) {
  NetworkServer *q;
  Driver::ptr driver;
  ImageHandler::ptr handler;
  NetworkDispatcher::ptr dispatcher;
  FramesForwarder::ptr framesForwarder;
  unique_ptr<QTcpServer> server;
  DriverForwarder::ptr forwarder;
  FilesystemForwarder::ptr filesystemForwarder;
  void new_connection();
  void bytes_sent(quint64 written, quint64 sent);
  QElapsedTimer elapsed;
  quint64 last_sent;
  bool paused = false;
};

NetworkServer::NetworkServer(const Driver::ptr &driver, const ImageHandler::ptr &handler, const NetworkDispatcher::ptr &dispatcher, const SaveFileForwarder::ptr &save_file, const FramesForwarder::ptr &framesForwarder, QObject* parent)
  : QObject{parent}, NetworkReceiver{dispatcher}, dptr(this, driver, handler, dispatcher, framesForwarder, make_unique<QTcpServer>())
{
  d->server->setMaxPendingConnections(1);
  d->filesystemForwarder = make_shared<FilesystemForwarder>(dispatcher);
  connect(d->server.get(), &QTcpServer::newConnection, bind(&Private::new_connection, d.get()));
  d->forwarder = make_shared<DriverForwarder>(dispatcher, driver, handler, [this, save_file](Imager *imager) {
    save_file->setImager(imager);
    emit imagerConnected(imager);
  });
  register_handler(NetworkProtocol::Hello, [this](const NetworkPacket::ptr &p){
    DriverProtocol::setFormatParameters(NetworkProtocol::decodeHello(p));
    QVariantMap status;
    d->forwarder->getStatus(status);
    d->dispatcher->send(NetworkProtocol::packetHelloReply() << status);
  });
  register_handler(DriverProtocol::StartLive, [this](const NetworkPacket::ptr &){
      d->elapsed.restart();
  });
  connect(d->dispatcher.get(), &NetworkDispatcher::bytes, this, bind(&Private::bytes_sent, d.get(), _1, _2));
  connect(save_file.get(), &SaveFileForwarder::isRecording, framesForwarder.get(), &FramesForwarder::recordingMode);
}


NetworkServer::~NetworkServer()
{
}

void NetworkServer::listen(const QString& address, int port)
{
  qDebug() << "Listening on %1:%2"_q % address % port;
  d->server->listen(QHostAddress{address}, port);
}

void NetworkServer::Private::new_connection()
{
  auto socket = server->nextPendingConnection();
  QObject::connect(socket, &QTcpSocket::disconnected, q, [this, socket] {
    qDebug() << "Client disconnected";
    dispatcher->setSocket(nullptr);
  });
  dispatcher->setSocket(socket);
  socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
  
  q->wait_for_processed(NetworkProtocol::Hello);
}

namespace {
    double to_kb(uint64_t size) { return static_cast<double>(size) / 1024.; }
    double to_mb(uint64_t size) { return to_kb(size) / 1024.; }
    double to_secs(qint64 msecs){ return static_cast<double>(msecs)/ 1000.; }
}

void NetworkServer::Private::bytes_sent(quint64 written, quint64 sent)
{
    qDebug() << "written: %1 MB, sent: %2 MB, cached: %3 MB, rate: %4 MB/sec"_q
      % to_mb(written) % to_mb(sent) % to_mb(written - sent) % ( to_mb(sent - last_sent) / to_secs(elapsed.elapsed()) );
    elapsed.restart();
    last_sent = sent;
    if(written - sent > 100'000'000) {
      framesForwarder->setEnabled(false);
      paused = true;
    } else if(paused) {
      framesForwarder->setEnabled(true);
    }
}



#include "networkserver.moc"
