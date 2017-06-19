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

#ifndef FC2_EXCEPTION_H
#define FC2_EXCEPTION_H

#include <Error.h>
#include <FlyCapture2Defs.h>
#include <QDebug>

#include "drivers/imagerexception.h"


class FC2Exception: public Imager::exception {
public:
    FC2Exception(int code, const std::string &message): Imager::exception(code, message) { }
};

#define FC2_CHECK C_ERROR_CHECK(std::not_equal_to<int>, FC2Exception, FlyCapture2::PGRERROR_OK)

#endif // FC2_EXCEPTION_H
