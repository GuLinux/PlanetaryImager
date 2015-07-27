/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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
#include "dptr.h"
#include <QObject>

class QHYCCDImager : public QObject
{
  Q_OBJECT
public:
    QHYCCDImager(QHYDriver::Camera camera);
    ~QHYCCDImager();
    struct Chip {
      double width, height, pixelwidth, pixelheight;
      int xres, yres, bpp;
    };
    QString name() const;
    QString id() const;
    Chip chip() const;

    struct Setting {
      int id;
      QString name;
      double min, max, step, value;
    };
    typedef QList<Setting> Settings;
    Settings settings() const;  
signals:
  void settingsLoaded(const Settings settings);
  void gotImage(const QImage &);
public slots:
  void setSetting(const Setting &setting);
  void startLive();
  void stopLive();
private:
  D_PTR
};

typedef std::shared_ptr<QHYCCDImager> QHYCCDImagerPtr;

QDebug operator<<(QDebug dbg, const QHYCCDImager::Chip &chip);
QDebug operator<<(QDebug dbg, const QHYCCDImager::Setting &setting);

#endif // QHYCCD_H
