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

#include <opencv2/tracking.hpp>
#include <QDebug>
#include <mutex>
#include <vector>

#include "tracking.h"


constexpr QPoint rectCenter(const cv::Rect2d &r)
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
    std::mutex guard; ///< Synchronizes accesses to 'targets'
    std::vector<Target> targets;
    cv::Ptr<cv::Tracker> tracker;
    Frame::const_ptr prevFrame;
};

#define LOCK()  std::lock_guard<std::mutex> lock(d->guard)


ImgTracker::ImgTracker(): dptr()
{
    #if (CV_MINOR_VERSION < 3)
    d->tracker = cv::Tracker::create("MIL");// Can't use "BOOSTING", as it does not work with grayscale frames.
    #else
    d->tracker = cv::TrackerKCF::create();
    #endif
}


ImgTracker::~ImgTracker()
{
}


void ImgTracker::addTarget(const QPoint &pos)
{
    if (!d->prevFrame)
    {
        qWarning() << "Attempted to set tracking target before any image has been received";
        return;
    }

    constexpr unsigned BBOX_SIZE = 32; //TODO: make it configurable

    Target newTarget{cv::Rect2d(pos.x() - BBOX_SIZE/2, pos.y() - BBOX_SIZE/2,
                                BBOX_SIZE, BBOX_SIZE)};

    LOCK();
    d->tracker->init(d->prevFrame->mat(), newTarget.bbox);
    d->targets.push_back(newTarget);
}


void ImgTracker::doHandle(Frame::const_ptr frame)
{
    d->prevFrame = frame;

    LOCK();
    for (auto &target: d->targets)
    {
        //Too slow!   d->tracker->update(frame->mat(), target.bbox);

        //TESTING #######
//        const auto c = rectCenter(target.bbox);
//        std::cout << "New pos is: " << c.x() << ", " << c.y() << std::endl;
    }
}


void ImgTracker::clear()
{
    d->targets.clear();
}


QPoint ImgTracker::getTargetPos(size_t index)
{
    LOCK();
    return rectCenter(d->targets.at(index).bbox);
}
