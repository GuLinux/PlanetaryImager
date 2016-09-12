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

#ifndef ZWOEXCEPTION_H
#define ZWOEXCEPTION_H

#include <stdexcept>
#include "ASICamera2.h"
#include "c++/dptr.h"
#include "drivers/imagerexception.h"

class ZWOException : public Imager::exception {
public:
  ZWOException(int code, const std::string &where = {});
};

#define ASI_CHECK C_ERROR_CHECK(std::less<int>, ZWOException, ASI_SUCCESS)

#endif // ZWOEXCEPTION_H
