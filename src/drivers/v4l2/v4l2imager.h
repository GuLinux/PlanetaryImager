/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <marco.gulino@bhuman.it>
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
 V4L2Imager(const QString &name, int index, const ImageHandlerPtr &handler);
 ~V4L2Imager();
 virtual Imager::Chip chip() const;
 virtual QString name() const;
 virtual Imager::Settings settings() const;

public slots:
  virtual void setSetting(const Setting &setting);
  virtual void startLive();
  virtual void stopLive();
private:
    DPTR
};

#endif // V4L2IMAGER_H
