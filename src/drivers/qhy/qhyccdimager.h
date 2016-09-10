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

#ifndef QHYCCD_H
#define QHYCCD_H
#include "qhydriver.h"
#include <drivers/imager.h>
#include "dptr.h"
#include <QObject>
#include <QList>

class QHYCCDImager : public Imager
{
  Q_OBJECT
public:
    QHYCCDImager(const QString &cameraName, const char *id, const ImageHandlerPtr &imageHandler);
    ~QHYCCDImager();

    virtual QString name() const;
    virtual Properties chip() const;

    virtual Controls controls() const;  
    virtual bool supportsROI();
public slots:
  virtual void setControl(const Control &setting);
  virtual void startLive();
  virtual void stopLive();
    virtual void setROI(const QRect &);
    virtual void clearROI();
private:
  DPTR
};




#endif // QHYCCD_H
