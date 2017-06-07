/*
 * Copyright (C) 2017 Filip Szczerek <ga.software@yahoo.com>
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

#ifndef IIDC_EXCEPTION_H
#define IIDC_EXCEPTION_H

#include "drivers/imagerexception.h"


class IIDCException: public Imager::exception {
public:
    IIDCException(int code, const std::string &message): exception(0, message) { }
};

#define IIDC_CHECK C_ERROR_CHECK(std::not_equal_to<int>, IIDCException, DC1394_SUCCESS)

#endif // IIDC_EXCEPTION_H
