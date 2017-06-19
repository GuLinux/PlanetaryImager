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

#include <algorithm>
#include <FlyCapture2.h>
#include <memory>
#include <vector>

#include "fc2_driver.h"
#include "fc2_exception.h"
#include "fc2_imager.h"


class FC2Camera: public Driver::Camera
{
    FlyCapture2::PGRGuid guid;
    QString camName;

public:

    typedef std::shared_ptr<FC2Camera> ptr;

    FC2Camera(const FlyCapture2::PGRGuid &_guid);

    virtual ~FC2Camera() { }

    Imager *imager(const ImageHandler::ptr &imageHandler) const override;

    QString name() const override { return camName; }
};

FC2Camera::FC2Camera(const FlyCapture2::PGRGuid &_guid): guid(_guid)
{
    FlyCapture2::Camera cam;
    // Connect() most likely does not modify the passed GUID
    FC2_CHECK << cam.Connect(const_cast<FlyCapture2::PGRGuid*>(&guid)).GetType()
              << "Camera::Connect";

    FlyCapture2::CameraInfo camInfo;
    FC2_CHECK << cam.GetCameraInfo(&camInfo).GetType()
              << "Camera::GetCameraInfo";

    camName = camInfo.modelName;
    if (camInfo.interfaceType == FlyCapture2::INTERFACE_IEEE1394 ||
        camInfo.interfaceType == FlyCapture2::INTERFACE_USB2 ||
        camInfo.interfaceType == FlyCapture2::INTERFACE_USB3)
    {
        // Appending driver name, because FireWire & USB cameras can be also detected & enumerated via IIDC driver
        camName += " (FlyCapture 2)";
    }
}

DPTR_IMPL(FC2Driver)
{
    FlyCapture2::BusManager busManager;
};

Imager *FC2Camera::imager(const ImageHandler::ptr &imageHandler) const
{
    return new FC2Imager(guid, imageHandler);
}

Driver::Cameras FC2Driver::cameras() const
{
    Driver::Cameras result;

    unsigned int numCams = 0;
    FC2_CHECK << d->busManager.GetNumOfCameras(&numCams).GetType()
              << "BusManager::GetNumOfCameras";

    for (auto i = 0; i < numCams; i++)
    {
        FlyCapture2::PGRGuid guid;
        FC2_CHECK << d->busManager.GetCameraFromIndex(i, &guid).GetType()
                  << "BusManager::GetCameraFromIndex";

        result.push_back(std::make_shared<FC2Camera>(guid));
    }

    return result;
}

FC2Driver::FC2Driver(): dptr()
{ }

FC2Driver::~FC2Driver()
{ }
