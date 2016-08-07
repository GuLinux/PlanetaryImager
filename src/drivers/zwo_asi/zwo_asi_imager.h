/*
 * Copyright (C) 2016 Marco Gulino (marco AT gulinux.net)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
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
    ZWO_ASI_Imager(const ASI_CAMERA_INFO &info, const ImageHandlerPtr &imageHandler);
    ~ZWO_ASI_Imager();
    virtual Imager::Chip chip() const;
    virtual QString name() const;
    virtual Imager::Settings settings() const;
    virtual bool supportsROI();
public slots:
    virtual void setSetting(const Setting &setting);
    virtual void startLive();
    virtual void stopLive();
    virtual void setROI(const QRect &);
    virtual void clearROI();
private:
    DPTR
};

#endif // ZWO_ASI_IMAGER_H
