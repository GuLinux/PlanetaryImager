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
  
protected:
  void restart(const ImagerThread::Worker::factory &worker);
  std::shared_ptr<QWaitCondition> push_job_on_thread(const ImagerThread::Job &job);
  void update_exposure();
  void wait_for(const std::shared_ptr<QWaitCondition> &wait_condition) const;
  // Call this at the end of each subclass constructor. Still have to find a better way for it, but whatever...
private:
  DPTR
  
public slots:
  virtual void setROI(const QRect &) = 0;
  virtual void clearROI() = 0;
  virtual void setControl(const Imager::Control &control) = 0;
  virtual void setControls(const Imager::Controls &controls);
  void import_controls(const QVariantList &controls, bool by_id = true);
  virtual void startLive() = 0;
  virtual void destroy();
  QVariantList export_controls() const;
  virtual void readTemperature();
signals:
  void fps(double rate);
  void temperature(double celsius);
  void changed(const Imager::Control &control);
  void disconnected();
  void exposure_changed(const Imager::Control &control);
  void long_exposure_started(double exposure_seconds);
  void long_exposure_ended();
};

struct Imager::Control {
  enum Type { Number, Combo, Bool };
  struct Choice {
    QString label;
    QVariant value;
  };
  typedef QList<Choice> Choices;
  struct Range {
    QVariant min, max, step;
  };
  
  qlonglong id;
  QString name;
  Type type = Number;
  QVariant value, default_value;
  Range range;
  Choices choices;
  qint16 decimals = 2;
  bool is_duration = false;
  bool supports_auto = false;
  bool value_auto = false;
  bool readonly = false;
  std::chrono::duration<double> duration_unit;
  bool is_exposure = false;
  bool supports_onOff = false;
  bool value_onOff = false;
  
  Control &set_id(const qlonglong &id) { this->id = id; return *this; }
  Control &set_name(const QString &name) { this->name= name; return *this; }
  Control &set_type(const Type &type) { this->type= type; return *this; }
  template<typename T> Control &set_value(const T &value) { this->value.setValue(value); return *this; }
  template<typename T> Control &set_default_value(const T &value) { this->default_value.setValue(value); return *this; }
  template<typename T> Control &set_range(const T &min, const T &max, const T &step) {
    this->range.min.setValue(min);
    this->range.max.setValue(max);
    this->range.step.setValue(step);
    return *this;
  }
  template<typename T> Control &add_choice(const QString &label, const T &value) {
    Choice choice{label};
    choice.value.setValue(value);
    choices.push_back(choice);
    return *this;
  }
  
  // TODO: handle enums in a smarter way? more templates?
  template<typename T> T get_value_enum() const { return static_cast<T>(value.value<int>()); }
  template<typename T> Control &set_default_value_enum(const T &value) { return set_default_value(static_cast<int>(value)); }
  template<typename T> Control &set_value_enum(const T &value) { return set_value(static_cast<int>(value)); }
  template<typename T> Control &add_choice_enum(const QString &label, const T &value) {
    return add_choice(label, static_cast<int>(value));
  }
  
  Control &set_decimals(const qint16 &decimals) { this->decimals = decimals; return *this; }
  Control &set_is_duration(bool is_duration) { this->is_duration= is_duration; return *this; }
  Control &set_supports_auto(bool supports_auto) { this->supports_auto= supports_auto; return *this; }
  Control &set_value_auto(bool value_auto) { this->value_auto= value_auto; return *this; }
  Control &set_readonly(bool readonly) { this->readonly= readonly; return *this; }
  Control &set_is_exposure(bool is_exposure) { this->is_exposure = is_exposure; return *this; }
  Control &set_duration_unit(double duration_unit) { this->duration_unit = std::chrono::duration<double>{duration_unit}; return *this; }
  Control &set_duration_unit(std::chrono::duration<double> duration_unit) { this->duration_unit = duration_unit; return *this; }
  
  template<typename T> T get_value() const { return value.value<T>(); }
  
  bool valid() const;
  inline std::chrono::duration<double> seconds() const { return duration_unit * value.toDouble(); }
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
    QSize resolution() const;
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
