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

#ifndef SERWRITER_H
#define SERWRITER_H

#include "filewriter.h"
#include "c++/dptr.h"

class SERWriter : public FileWriter
{
public:
 SERWriter(const QString &deviceName, const Configuration &configuration);
 ~SERWriter();
 QString filename() const override;

private:

    void doHandle(Frame::const_ptr frame) override;

   DPTR
};

#endif // SERWRITER_H
