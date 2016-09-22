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
#ifndef PLANETARYIMAGER_V4L2_UTILS_H
#define PLANETARYIMAGER_V4L2_UTILS_H

inline QString FOURCC2QS(int32_t _4cc)
{
    auto get_byte = [=](int b) { return static_cast<char>( _4cc >> b & 0xff ); };
    char data[5] { get_byte(0), get_byte(8), get_byte(0x10), get_byte(0x18), '\0' };
    return {data};
}

#endif
