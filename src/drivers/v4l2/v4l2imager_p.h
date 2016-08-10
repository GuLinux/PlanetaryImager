#ifndef V4L2_IMAGER_P_H
#define V4L2_IMAGER_P_H


#include "v4l2imager.h"
#include <linux/videodev2.h>
#include <QDebug>
#include "fps_counter.h"
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "Qt/strings.h"
#include "Qt/functional.h"
#include <sys/mman.h>
#include "drivers/imagerthread.h"

#define PIXEL_FORMAT_CONTROL_ID -10
#define RESOLUTIONS_CONTROL_ID -9
#define FPS_CONTROL_ID -8

class V4L2Device;


struct V4LBuffer {
  typedef std::shared_ptr<V4LBuffer> ptr;
    v4l2_buffer bufferinfo;
    char *memory;
    V4LBuffer(int index, const std::shared_ptr<V4L2Device> &v4ldevice);
    ~V4LBuffer();
    std::shared_ptr<V4L2Device> v4ldevice;
    void queue();
    class List : public QList<ptr> {
    public:
        std::shared_ptr< V4LBuffer > dequeue(const std::shared_ptr< V4L2Device >& device) const;
    };
};



DPTR_IMPL(V4L2Imager)
{
    class Worker;
    ImageHandlerPtr handler;
    const QString device_path;
    V4L2Imager *q;
    
    std::shared_ptr<V4L2Device> device;
    ImagerThread::ptr imager_thread;

    QList<v4l2_fmtdesc> formats() const;
    v4l2_format query_format() const;
    QList<v4l2_frmsizeenum> resolutions(const v4l2_format &format) const;
    void adjust_framerate(const v4l2_format &format) const;
    
    struct V4lSetting {
      Imager::Setting setting;
      int querycode;
      int valuecode;
      bool disabled;
      bool unknown_type;
      operator bool() const { return querycode != -1 && valuecode != -1 && !disabled && !unknown_type; }
    };
    V4lSetting setting(uint32_t id);
    void open_camera();
    QString driver, bus, cameraname;
    QString dev_name;
    typedef std::function<void(Setting &)> SettingRule;
    QList<SettingRule> setting_rules;
    void populate_rules();
};

class V4L2Imager::Private::Worker : public ImagerThread::Worker {
public:
  Worker(V4L2Imager::Private *d);
  virtual bool shoot(const ImageHandlerPtr& imageHandler);
  virtual void start();
  virtual void stop();
  V4L2Imager::Private *d;
  V4LBuffer::List buffers;
  v4l2_format format;
  int bufferinfo_type;
};

class V4L2Device {
public:
  V4L2Device(const QString &path);
  ~V4L2Device();
  inline QString path() const { return _path; }
  inline operator bool() const { return fd != -1; }
  int descriptor() const { return fd; }
  template<typename T> void ioctl(uint64_t ctl, T *data, const QString &errorLabel = {}) const;
  template<typename T> int xioctl(uint64_t ctl, T *data, const QString &errorLabel = {}) const;
  class exception : public std::exception {
  public:
     exception(const QString &label = {}) : label{label}, _error_code{errno} {}
     virtual const char* what() const noexcept;
     int error_code() const { return _error_code; }
  private:
    const QString label;
    int _error_code;
  };
private:
  int fd = -1;
  const QString _path;
};



inline QString FOURCC2QS(int32_t _4cc)
{
    auto get_byte = [=](int b) { return static_cast<char>( _4cc >> b & 0xff ); };
    char data[5] { get_byte(0), get_byte(8), get_byte(0x10), get_byte(0x18), '\0' };
    return {data};
}

inline QDebug operator<<(QDebug dbg, v4l2_fract frac) {
    dbg.nospace() << frac.numerator << "/" << frac.denominator;
    return dbg.space();
};


inline QDebug operator<<(QDebug dbg, const v4l2_frmivalenum &fps_s) {
    dbg.nospace() << "v4l2_frmivalenum{ index=" << fps_s.index << ", " << fps_s.width << "x" << fps_s.height << ", 4cc=" << FOURCC2QS(fps_s.pixel_format);
    if(fps_s.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
        dbg << "discrete: " << fps_s.discrete;
    }
    if(fps_s.type == V4L2_FRMIVAL_TYPE_STEPWISE) {
        dbg << "stepwise: min=" << fps_s.stepwise.min << ", max=" << fps_s.stepwise.max << ", step=" << fps_s.stepwise.step;
    }
    if(fps_s.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {
        dbg << "continuous" << fps_s.stepwise.min << ", max=" << fps_s.stepwise.max << ", step=" << fps_s.stepwise.step;
    }
    dbg << " }";
    return dbg.space();
}

#endif
