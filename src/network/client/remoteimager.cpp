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

#include "remoteimager.h"

using namespace std;

DPTR_IMPL(RemoteImager) {
};

RemoteImager::RemoteImager(qlonglong id, const ImageHandler::ptr& image_handler, const NetworkDispatcher::ptr &dispatcher) : Imager{image_handler}, dptr()
{
}

RemoteImager::~RemoteImager()
{
}


void RemoteImager::startLive()
{
}

void RemoteImager::clearROI()
{
}

Imager::Controls RemoteImager::controls() const
{
}

QString RemoteImager::name() const
{
}

Imager::Properties RemoteImager::properties() const
{
}

void RemoteImager::setControl(const Imager::Control& control)
{
}

void RemoteImager::setROI(const QRect&)
{
}


#include "remoteimager.moc"
