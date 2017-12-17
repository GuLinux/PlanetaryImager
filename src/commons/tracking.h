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

#ifndef TRACKING_H
#define TRACKING_H

#include <QtCore/qpoint.h>
#include <opencv2/tracking.hpp>
#include <vector>

#include "commons/frame.h"


class ImgTracker
{
    DPTR_CUST_DEL

public:

    ImgTracker();

    /// Adds new target to track
    void addTarget(const QPoint &pos, Frame::const_ptr initialImg);

    /// Updates positions of targets using the specified new image
    void updatePositions(Frame::const_ptr newImg);

    /// Cancels tracking and removes all targets
    void clear();

    /// Returns target's current position
    QPoint getTargetPos(size_t index); //TODO: indicate "not updated"

    //TODO: "target lost" signal
};


#endif // TRACKING_H
