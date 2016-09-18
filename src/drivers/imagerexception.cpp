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
#include "imagerexception.h"
#include <sstream>
using namespace std;
DPTR_IMPL(Imager::exception) {
    int code;
    string what;
};

Imager::exception::exception(int code, const unordered_map<int, string> codes_map, const string& message_prefix, const string& where)
    : dptr(code)
{
  ostringstream message;
  message << message_prefix << (codes_map.count(code) ? codes_map.at(code) : "Unknown error code") << " (code: " << code << ")";
  if(!where.empty())
    message << " on " << where;
  d->what = message.str();
}

Imager::exception::exception(const Imager::exception& other) : dptr(other.d->code, other.d->what)
{
}

const char * Imager::exception::what() const noexcept
{
    return d->what.c_str();
}

int Imager::exception::code() const
{
    return d->code;
}

Imager::exception::~exception()
{
}
