#ifndef PL_IMG_IMAGER_H
#define PL_IMG_IMAGER_H

#include <memory>
#include <QObject>
#include <QDebug>
#include "imagehandler.h"

class Imager : public QObject {
  Q_OBJECT
public:
  Imager() : QObject(nullptr) {}
  struct Setting {
    int64_t id;
    QString name;
    double min, max, step, value, defaut_value;
    enum Type { Number, String, Combo, Bool } type;
    struct Choice {
     QString label;
     double value;
    };
    QList<Choice> choices;
    operator bool() const;
  };
  typedef QList<Setting> Settings;

    virtual Settings settings() const = 0;  
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
  virtual void setSetting(const Setting &setting) = 0;
  virtual void startLive() = 0;
  virtual void stopLive() = 0;
signals:
  void fps(double rate);
  void changed(const Setting &setting);
  void disconnected();
};

typedef std::shared_ptr<Imager> ImagerPtr;
QDebug operator<<(QDebug dbg, const Imager::Chip &chip);
QDebug operator<<(QDebug dbg, const Imager::Setting &setting);
QDebug operator<<(QDebug dbg, const Imager::Setting::Choice &choice);

Q_DECLARE_METATYPE(Imager::Setting)
#endif
