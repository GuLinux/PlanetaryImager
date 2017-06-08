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

#ifndef IIDC_DRIVER_H
#define IIDC_DRIVER_H

#include "drivers/driver.h"


class IIDCDriver: public Driver
{
    Q_OBJECT

public:

    DECLARE_DRIVER_PLUGIN

    IIDCDriver();

    ~IIDCDriver();

    Driver::Cameras cameras() const override;

private:

    DPTR
};

#endif // IIDC_DRIVER_H
