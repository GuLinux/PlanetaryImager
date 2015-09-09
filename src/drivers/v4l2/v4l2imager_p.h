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

class V4L2Imager::Private
{
public:
    Private(const ImageHandlerPtr &handler, V4L2Imager *q);
    ImageHandlerPtr handler;
    int v4l_fd;
    bool live = false;
    GuLinux::Thread *live_thread = nullptr;
    
    v4l2_format query_format() const;
    QList<v4l2_frmsizeenum> resolutions() const;
    static int ioctl(int fh, int request, void *arg);
    
    struct V4lSetting {
      Imager::Setting setting;
      int querycode;
      int valuecode;
      bool disabled;
      bool unknown_type;
      operator bool() const { return querycode != -1 && valuecode != -1 && !disabled && !unknown_type; }
    };
    V4lSetting setting(int id);
    QString driver, bus, cameraname;

private:
    V4L2Imager *q;
};



#endif
