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

#ifndef PL_IMG_IMAGER_H
#define PL_IMG_IMAGER_H

#include <memory>
#include <chrono>
#include <QObject>
#include <QDebug>
#include "imagehandler.h"

class Imager : public QObject {
  Q_OBJECT
public:
  Imager() : QObject(nullptr) {}
  struct Control {
    int64_t id;
    QString name;
    double min, max, step, value, defaut_value;
    enum Type { Number, Combo, Bool } type;
    struct Choice {
     QString label;
     double value;
    };
    QList<Choice> choices;
    operator bool() const;
    int decimals = 2;
    bool is_duration = false;
    std::chrono::duration<double> duration_unit;
    bool supports_auto = false;
    bool value_auto = false;
    bool readonly = false;
  };
  typedef QList<Control> Controls;

  virtual Controls controls() const = 0;  
  struct Chip {
    double width, height, pixelwidth, pixelheight;
    uint32_t xres, yres, bpp;
    struct Property {
      QString name;
      QString value;
    };
    QList<Property> properties;
  };
  virtual QString name() const = 0;
  virtual Chip chip() const = 0;
  virtual bool supportsROI() = 0;
public slots:
  virtual void setROI(const QRect &) = 0;
  virtual void clearROI() = 0;
  virtual void setControl(const Control &control) = 0;
  virtual void startLive() = 0;
  virtual void stopLive() = 0;
signals:
  void fps(double rate);
  void changed(const Control &control);
  void disconnected();
};

typedef std::shared_ptr<Imager> ImagerPtr;
QDebug operator<<(QDebug dbg, const Imager::Chip &chip);
QDebug operator<<(QDebug dbg, const Imager::Control &setting);
QDebug operator<<(QDebug dbg, const Imager::Control::Choice &choice);

Q_DECLARE_METATYPE(Imager::Control)
#endif
