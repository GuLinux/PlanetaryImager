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
#include <QSet>
#include "image_handlers/imagehandler.h"
#include "imagerthread.h"
#include "c++/dptr.h"
#include <QWaitCondition>

class Imager : public QObject {
  Q_OBJECT
public:
  class exception;
  Imager(const ImageHandler::ptr &image_handler);
  virtual ~Imager();
  struct Control;
  struct Properties;
  enum Capability { ROI, Temperature, LiveStream, StillPicture, };
  typedef QList<Control> Controls;

  virtual Controls controls() const = 0;  
  virtual QString name() const = 0;
  virtual Properties properties() const = 0;
  bool supports(Capability capability) const;
  QVariantList export_controls() const;
  
protected:
  void restart(const ImagerThread::Worker::factory &worker);
  void push_job_on_thread(const ImagerThread::Job &job);
  void set_exposure(const Control &control);
private:
  DPTR
  
public slots:
  virtual void setROI(const QRect &) = 0;
  virtual void clearROI() = 0;
  virtual std::shared_ptr<QWaitCondition> setControl(const Control &control) = 0;
  void import_controls(const QVariantList &controls, bool by_id = true);
  virtual void startLive() = 0;
  virtual void destroy();
signals:
  void fps(double rate);
  void temperature(double celsius);
  void changed(const Control &control);
  void disconnected();
  void long_exposure_started(double exposure_seconds);
  void long_exposure_ended();
};

struct Imager::Control {
  int64_t id;
  QString name;
  double min, max, step, value, default_value;
  enum Type { Number, Combo, Bool } type;
  struct Choice {
    QString label;
    double value;
  };
  QList<Choice> choices;
  bool valid() const;
  int decimals = 2;
  bool is_duration = false;
  std::chrono::duration<double> duration_unit;
  inline std::chrono::duration<double> seconds() const { return duration_unit * value; }
  bool supports_auto = false;
  bool value_auto = false;
  bool readonly = false;
  bool same_value(const Control &other) const;
  QVariantMap asMap() const;
  void import(const QVariantMap &data, bool full_import = false);
};

struct Imager::Properties {
    Properties &set_resolution_pixelsize(const QSize &resolution, double pixelwidth, double pixelheight);
    Properties &set_resolution_chipsize(const QSize &resolution, double width, double height);
    Properties &set_pixelsize_chipsize(double pixelwidth, double pixelheight, double width, double height);
    Properties &set_resolution(const QSize &resolution);
    Properties &set_pixel_size(double width, double height);
    Properties &set_chip_size(double width, double height);
//   double width, height, pixelwidth, pixelheight;
//   uint32_t xres, yres, bpp;
  struct Property {
    QString name;
    QVariant value;
    QString display_name;
    QString display_value;
    
    bool hidden = false;
    QString displayName() const;
    QString displayValue() const;
  };
  Properties &operator<<(const Property &property);
  QList<Property> properties;
  QSet<Capability> capabilities;
  Properties &operator<<(Capability capability);
  Properties &support(Capability capability);
};

QDebug operator<<(QDebug dbg, const Imager::Properties &chip);
QDebug operator<<(QDebug dbg, const Imager::Properties::Property &property);
QDebug operator<<(QDebug dbg, const Imager::Control &setting);
QDebug operator<<(QDebug dbg, const Imager::Control::Choice &choice);


Q_DECLARE_METATYPE(Imager::Control)
#endif
