/*
 * Copyright (C) 2017 Filip Szczerek <ga.software@yahoo.com>
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

#include <C/FlyCapture2_C.h>

#include "fc2_driver.h"
#include "fc2_exception.h"
#include "fc2_imager.h"


class FC2Camera: public Driver::Camera
{
    fc2PGRGuid guid;
    QString camName;

public:

    typedef std::shared_ptr<FC2Camera> ptr;

    FC2Camera(fc2Context context, const fc2PGRGuid &_guid);

    virtual ~FC2Camera();

    Imager *imager(const ImageHandlerPtr &imageHandler) const override;

    QString name() const override { return camName; }
};

FC2Camera::FC2Camera(fc2Context context, const fc2PGRGuid &_guid): guid(_guid)
{
    FC2_CHECK << fc2Connect(context, const_cast<fc2PGRGuid*>(&guid))
              << "fc2Connect";

    fc2CameraInfo camInfo;
    FC2_CHECK << fc2GetCameraInfo(context, &camInfo)
              << "fc2GetCameraInfo";

    camName = camInfo.modelName;
    if (camInfo.interfaceType == FC2_INTERFACE_IEEE1394 ||
        camInfo.interfaceType == FC2_INTERFACE_USB_2 ||
        camInfo.interfaceType == FC2_INTERFACE_USB_3)
    {
        // Appending driver name, because FireWire & USB cameras can be also detected & enumerated via IIDC driver
        camName += " (FlyCapture 2)";
    }

    FC2_CHECK << fc2Disconnect(context)
              << "fc2Disconnect";
}

FC2Camera::~FC2Camera() { }

DPTR_IMPL(FC2Driver)
{
};

Imager *FC2Camera::imager(const ImageHandlerPtr &imageHandler) const
{
    return new FC2Imager(guid, imageHandler);
}

Driver::Cameras FC2Driver::cameras() const
{
    fc2Context context;
    FC2_CHECK << fc2CreateContext(&context)
              << "fc2CreateContext";

    Driver::Cameras result;

    unsigned int numCams;
    FC2_CHECK << fc2GetNumOfCameras(context, &numCams)
              << "fc2GetNumOfCameras";

    for (auto i = 0; i < numCams; i++)
    {
        fc2PGRGuid guid;
        FC2_CHECK << fc2GetCameraFromIndex(context, i, &guid)
                  << "fc2GetCameraFromIndex";

        result.push_back(std::make_shared<FC2Camera>(context, guid));
    }

    FC2_CHECK << fc2DestroyContext(context)
              << "fc2CreateContext";

    return result;
}

FC2Driver::FC2Driver(): dptr()
{
}

FC2Driver::~FC2Driver()
{
}

DECLARE_DRIVER_PLUGIN_INIT(FC2Driver)
