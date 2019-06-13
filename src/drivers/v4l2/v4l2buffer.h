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

#ifndef V4LBUFFER_H
#define V4LBUFFER_H
#include "c++/dptr.h"
#include <linux/videodev2.h>
#include <QList>
#include "commons/fwd.h"

FWD_PTR(V4L2Device)
FWD_PTR(V4LBuffer)

class V4LBuffer
{
public:
    V4LBuffer(int index, const V4L2DevicePtr &v4ldevice);
    ~V4LBuffer();
    void queue();
    class List : public QList<V4LBufferPtr> {
    public:
        std::shared_ptr< V4LBuffer > dequeue(const V4L2DevicePtr& device) const;
    };
    char *bytes() const;
    uint32_t type() const;
    uint32_t size() const;
private:
  DPTR
};

#endif // V4LBUFFER_H
