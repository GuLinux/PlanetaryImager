/*
 * Copyright (C) 2018 Filip Szczerek <ga.software@yahoo.com>
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

#ifndef MOUNT_H
#define MOUNT_H

#include <string>
#include <vector>


namespace Mount
{

enum class ConnectionType
{
    INDI,      ///< INDI server
    SkyWatcher ///< Sky-Watcher direct serial connection
};

bool isConnectionSupported(ConnectionType connType);

std::vector<std::string> getIndiDevices(const char *hostname, unsigned port);


} // namespace Mount

#endif // MOUNT_H
