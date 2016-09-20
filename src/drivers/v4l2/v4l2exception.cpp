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


#include "v4l2exception.h"
#include <unordered_map>
#include "c++/stringbuilder.h"

using namespace std;
using namespace GuLinux;

V4L2Exception::V4L2Exception(int retcode, const string &where)
  : Imager::exception{
      errno,
      string{strerror(errno)}, 
    GuLinux::stringbuilder() << "V4L2 error (" << retcode << ") ",
    where
  }
{
}

