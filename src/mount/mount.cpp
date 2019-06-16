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

#include "mount.h"
#include "commons/definitions.h"

namespace Mount
{

bool isConnectionSupported(ConnectionType connType)
{
    switch (connType)
    {
        case ConnectionType::INDI:
            return HAVE_LIBINDI == 1;
        case ConnectionType::SkyWatcher:
            return false;
        // safety net, returning false as driver is unknown
        default:
            return false;
    }
}

} // namespace Mount
