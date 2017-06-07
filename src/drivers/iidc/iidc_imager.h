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

#ifndef IIDC_IMAGER_H
#define IIDC_IMAGER_H

#include <dc1394/dc1394.h>
#include "iidc_deleters.h"
#include "drivers/imager.h"
#include "c++/dptr.h"


class QRect;

class IIDCImager: public Imager
{
    Q_OBJECT

public:

    IIDCImager(std::unique_ptr<dc1394camera_t, Deleters::camera> camera, const ImageHandler::ptr &handler,
               const QString &cameraName, const QString &cameraVendor);

    virtual ~IIDCImager();

    Properties properties() const override;

    QString name() const override;

    Controls controls() const override;

    void startLive() override;


public slots:

    void setROI(const QRect &) override;

    void clearROI() override;

    void setControl(const Control& control) override;

    void readTemperature() override;

private:

    DPTR
};


#endif // IIDC_IMAGER_H
