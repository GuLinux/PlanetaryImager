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

#include "zwo_asi_driver.h"
#include "ASICamera2.h"
#include "zwo_asi_imager.h"
#include "zwoexception.h"

using namespace std;

DPTR_IMPL(ZWO_ASI_Driver) {
  ZWO_ASI_Driver *q;
};

namespace {
  class ZWO_ASI_Camera : public Driver::Camera {
  public:
    typedef shared_ptr<ZWO_ASI_Camera> ptr;
    ZWO_ASI_Camera(const ASI_CAMERA_INFO &info) : info{info} {}
    virtual Imager * imager(const ImageHandler::ptr& imageHandler) const;
    virtual QString name() const { return {info.Name};}
  private:
    const ASI_CAMERA_INFO info;
  };

}



Imager * ZWO_ASI_Camera::imager(const ImageHandler::ptr& imageHandler) const
{
  return new ZWO_ASI_Imager(info, imageHandler);
}


ZWO_ASI_Driver::ZWO_ASI_Driver() : dptr(this)
{
  static bool metatypes_registered = false;
  if(!metatypes_registered) {
    metatypes_registered = true;
    qRegisterMetaType<ASI_IMG_TYPE>("ASI_IMG_TYPE");
  }
}

ZWO_ASI_Driver::~ZWO_ASI_Driver()
{

}

Driver::Cameras ZWO_ASI_Driver::cameras() const
{
  int ncams = ASIGetNumOfConnectedCameras();
  Driver::Cameras cameras;
  int index=0;
  for(int index=0; index<ncams; index++) {
    ASI_CAMERA_INFO info;
    ASI_CHECK << ASIGetCameraProperty(&info, index++) << string{"Get Camera Property"};
    cameras.push_back(make_shared<ZWO_ASI_Camera>(info));
  }
  return cameras;
}
