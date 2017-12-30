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

#include <opencv2/core.hpp>
#include <QDebug>
#include <mutex>
#include <numeric>
#include <vector>

#include "tracking.h"


constexpr QPoint rectCenter(const cv::Rect2d &r)
{
    return { (int)(r.x + r.width/2),
             (int)(r.y + r.height/2) };
}


/// Finds new position of 'refBlock' in 'img' using block matching
QPoint findNewPos(const QPoint &oldPos, ///< Old position of 'refBlock's center in 'img'
                  cv::Mat refBlock,
                  cv::Mat img)
{
    // At first using a coarse step when trying to match 'refBlock'
    // with 'img' at different positions. Once an approximate matching
    // position is determined, the search continues around it repeatedly
    // using a smaller step, until the step becomes 1.
    unsigned searchStep = 4;
    const int searchRadius = 32; // TODO: make it configurable

    // Range of positions where 'refBlock' will be match-tested with 'img'.
    struct
    {
        int xmin, ymin; // inclusive
        int xmax, ymax; // exclusive
    } searchRange = { .xmin = oldPos.x() - searchRadius,
                      .ymin = oldPos.y() - searchRadius,
                      .xmax = oldPos.x() + searchRadius,
                      .ymax = oldPos.y() + searchRadius };

    const int rbwidth = refBlock.cols;
    const int rbheight = refBlock.rows;

    QPoint bestPos{ 0, 0 };

    cv::Mat absDiff = cv::Mat(refBlock.rows, refBlock.cols, refBlock.type());

    while (searchStep)
    {
        // Min. sum of absolute differences between pixel values of
        // the reference block and the image at candidate positions.
        double minDiffSum = DBL_MAX;

        // (x, y) = position in 'img' for which a block match test is performed
        for (int y = searchRange.ymin; y < searchRange.ymax;  y += searchStep)
            for (int x = searchRange.xmin; x < searchRange.xmax;  x += searchStep)
            {
                cv::absdiff(refBlock,
                            cv::Mat(img, cv::Rect{ x - rbwidth/2, y - rbheight/2, rbwidth, rbheight }),
                            absDiff);

                const auto sumAbsDiffChannels = cv::sum(absDiff);
                const double sumAbsDiffs = std::accumulate(sumAbsDiffChannels.val, sumAbsDiffChannels.val + 4, 0.0);

                if (sumAbsDiffs < minDiffSum)
                {
                    minDiffSum = sumAbsDiffs;
                    bestPos = QPoint{ x, y };
                }
            }

        searchRange.xmin = bestPos.x() - searchStep;
        searchRange.ymin = bestPos.y() - searchStep;
        searchRange.xmax = bestPos.x() + searchStep;
        searchRange.ymax = bestPos.y() + searchStep;

        searchStep /= 2;
    }

    return bestPos;
}


struct Target
{
    /// Target position; corresponds with the middle of 'refBlock'
    QPoint pos;
    cv::Mat refBlock; ///< Reference block
};


DPTR_IMPL(ImgTracker)
{
    std::mutex guard; ///< Synchronizes accesses to 'targets'
    std::vector<Target> targets;
    Frame::const_ptr prevFrame;
};

#define LOCK()  std::lock_guard<std::mutex> lock(d->guard)


ImgTracker::ImgTracker(): dptr()
{
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

    Target newTarget{ pos, cv::Mat(d->prevFrame->mat(), cv::Rect{ pos.x() - BBOX_SIZE/2,
                                                                  pos.y() - BBOX_SIZE/2,
                                                                  BBOX_SIZE, BBOX_SIZE }).clone() };

    LOCK();
    d->targets.push_back(newTarget);
}


void ImgTracker::doHandle(Frame::const_ptr frame)
{
    d->prevFrame = frame;

    LOCK();
    for (auto &target: d->targets)
        target.pos = findNewPos(target.pos, target.refBlock, frame->mat());
}


void ImgTracker::clear()
{
    d->targets.clear();
}


QPoint ImgTracker::getTargetPos(size_t index)
{
    LOCK();
    return d->targets.at(index).pos;
}
