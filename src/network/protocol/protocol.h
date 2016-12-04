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

#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H

#include <QString>

class NetworkProtocol {
public:
  typedef QString NameType;
};

#define ADD_PROTOCOL_NAME(name) static const NameType name
#define PROTOCOL_NAME__CUSTOM_VALUE(Area, name, value) const NameType Area ## Protocol::name = value
#define PROTOCOL_NAME_VALUE(Area, name) const NetworkProtocol::NameType Area ## Protocol::name = #Area "_" #name
#endif
