/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <marco.gulino@bhuman.it>
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

#include "v4l2imager.h"

class V4L2Imager::Private
{
public:
    Private(V4L2Imager *q);

private:
    V4L2Imager *q;
};

V4L2Imager::Private::Private(V4L2Imager *q) : q(q)
{
}

V4L2Imager::V4L2Imager(const QString &name, int index, const ImageHandlerPtr &handler)
    : dptr(this)
{
}

V4L2Imager::~V4L2Imager()
{
}

Imager::Chip V4L2Imager::chip() const
{
}

QString V4L2Imager::name() const
{
}

Imager::Settings V4L2Imager::settings() const
{
}

void V4L2Imager::setSetting(const Setting &setting)
{

}

void V4L2Imager::startLive()
{

}

void V4L2Imager::stopLive()
{

}

