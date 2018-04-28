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
#include <tuple>

#include "commons/frame.h"
#include "image_handlers/imagehandler.h"


/// Tracks image movement
/** Works in block matching or centroid mode. In case of block matching, multiple tracking targets
    may be specified (to protect against local disturbances, e.g. due to a passing bird/satellite).
    Centroid mode is best suited for planets.

    Tracking position is updated on every call to ImageHandler::doHandle().
*/
class ImgTracker: public QObject, public ImageHandler
{
    Q_OBJECT
    DPTR

public:

    enum class TrackingMode { Disabled, Centroid, BlockMatching };

    ImgTracker();
    ~ImgTracker();

    ImgTracker(const ImgTracker &)             = delete;
    ImgTracker(ImgTracker &&)                  = delete;
    ImgTracker &operator =(const ImgTracker &) = delete;
    ImgTracker &operator =(ImgTracker &&)      = delete;

    TrackingMode getTrackingMode() const;

    /// Sets the centroid calculation area
    /** Removes any block-matching targets. Returns 'false' on failure. */
    bool setCentroidCalcRect(const QRect &rect);

    /// Returns either the centroid position or the block matching targets' common position
    QPoint getTrackingPosition() const;

    /// Returns centroid calculation area and centroid position in the image
    std::tuple<QRect, QPoint> getCentroidAreaAndPos() const;

    /// Adds new target to track via block matching
    /** Cancels centroid tracking (if enabled). Returns 'false' on failure. */
    bool addBlockMatchingTarget(const QPoint &pos);

    /// Cancels tracking and removes all targets
    void clear();

    /// Returns current positions of block matching targets
    std::vector<QPoint> getBlockMatchingTargetPositions();

signals:
    /// Emitted when tracking target is lost (moves outside image or moves away too fast)
    void targetLost();

private:
    void doHandle(Frame::const_ptr frame) override;
};

#endif // TRACKING_H
