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
 */

#ifndef QHYEXCEPTION_H
#define QHYEXCEPTION_H

#include "drivers/imagerexception.h"

class QHYException : public Imager::exception
{
public:
  QHYException(int code, const std::string &where = {});
};

#define QHY_CHECK C_ERROR_CHECK(std::less<int>, QHYException, QHYCCD_SUCCESS)

#endif // QHYEXCEPTION_H
