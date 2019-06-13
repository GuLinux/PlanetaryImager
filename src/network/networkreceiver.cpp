/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
 * Copyright (C) 2019  Marco Gulino <marco@gulinux.net>
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

#include "network/networkdispatcher.h"
#include "network/networkreceiver.h"
#include "network/networkpacket.h"
#include <QCoreApplication>

using namespace std;

DPTR_IMPL(NetworkReceiver) {
  const NetworkDispatcherPtr dispatcher;
  QHash<NetworkPacketType, bool> packets_processed;
  QHash<NetworkPacketType, NetworkReceiver::HandlePacket> handlers;
};

NetworkReceiver::NetworkReceiver(const NetworkDispatcherPtr &dispatcher) : dptr(dispatcher)
{
  dispatcher->attach(this);
}

NetworkReceiver::~NetworkReceiver()
{
  d->dispatcher->detach(this);
}

NetworkDispatcherPtr NetworkReceiver::dispatcher() const
{
  return d->dispatcher;
}


void NetworkReceiver::wait_for_processed(const NetworkPacketType &name) const
{
  if(! d->dispatcher->is_connected())
    return;
  d->packets_processed[name] = false;
  while(! d->packets_processed[name] && d->dispatcher->is_connected())
    qApp->processEvents();
}



void NetworkReceiver::register_handler(const NetworkPacketType& name, const HandlePacket handler)
{
  d->handlers[name] = handler;
}

void NetworkReceiver::handle(const NetworkPacketPtr& packet)
{
  auto handler = d->handlers[packet->name()];
  if(handler)
    handler(packet);
  d->packets_processed[packet->name()] = true;
}



