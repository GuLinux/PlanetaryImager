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

#ifndef ZWO_ASI_IMAGER_H
#define ZWO_ASI_IMAGER_H

#include "drivers/imager.h"
#include "ASICamera2.h"
#include "dptr.h"

class ZWO_ASI_Imager : public Imager
{
public:
    ZWO_ASI_Imager(const ASI_CAMERA_INFO &info, const ImageHandler::ptr &imageHandler);
    ~ZWO_ASI_Imager();
    Imager::Properties properties() const override;
    QString name() const override;
    Imager::Controls controls() const override;
    bool supportsROI() const override;
public slots:
    void setControl(const Imager::Control& control) override;
    void startLive() override;
    void setROI(const QRect &) override;
    void clearROI() override;
private:
    DPTR
};

#endif // ZWO_ASI_IMAGER_H
