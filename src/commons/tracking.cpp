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

#include "tracking.h"


constexpr QPoint rect_center(const cv::Rect2d &r)
{
    return { (int)(r.x + r.width/2),
             (int)(r.y + r.height/2) };
}


struct Target
{
    cv::Rect2d bbox; ///< Bounding box
};


DPTR_IMPL(ImgTracker)
{
    std::vector<Target> targets;
    cv::Ptr<cv::Tracker> tracker;
};
DPTR_DEL(ImgTracker)


ImgTracker::ImgTracker(): dptr()
{
    #if (CV_MINOR_VERSION < 3)
    d->tracker = cv::Tracker::create("BOOSTING");
    #else
    d->tracker = cv::TrackerKCF::create();
    #endif

    std::cout << "Constructed!" << std::endl;
}


void ImgTracker::addTarget(const QPoint &pos, Frame::const_ptr initialImg)
{
    constexpr unsigned BBOX_SIZE = 32; //TODO: make it configurable

    Target newTarget{cv::Rect2d(pos.x() - BBOX_SIZE/2, pos.y() - BBOX_SIZE/2,
                                BBOX_SIZE, BBOX_SIZE)};

    d->tracker->init(initialImg->mat(), newTarget.bbox);
    d->targets.push_back(newTarget);
}


void ImgTracker::updatePositions(Frame::const_ptr newImg)
{
    for (auto &target: d->targets)
        d->tracker->update(newImg->mat(), target.bbox);
}


void ImgTracker::clear()
{
    d->targets.clear();
}


QPoint ImgTracker::getTargetPos(size_t index)
{
    return rect_center(d->targets.at(index).bbox);
}
