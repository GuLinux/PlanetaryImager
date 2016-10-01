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

#ifndef V4L2IMAGER_H
#define V4L2IMAGER_H

#include <drivers/imager.h>
#include "dptr.h"

class V4L2Imager : public Imager
{
public:
    V4L2Imager(const QString &name, int index, const ImageHandler::ptr &handler);
    ~V4L2Imager();
    Imager::Properties properties() const override;
    QString name() const override;
    Imager::Controls controls() const override;
    bool supportsROI() const override { return false; }
public slots:
    void setControl(const Control &setting) override;
    void startLive() override;
    void setROI(const QRect &) override {}
    void clearROI() override {}
private:
    DPTR
};

#endif // V4L2IMAGER_H
