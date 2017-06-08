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

#include <cassert>
#include <dc1394/dc1394.h>
#include "drivers/imagerexception.h"
#include "iidc_deleters.h"
#include "iidc_exception.h"
#include "iidc_driver.h"
#include "iidc_imager.h"
#include <memory>



DPTR_IMPL(IIDCDriver)
{
    std::unique_ptr<dc1394_t,            Deleters::dc1394>      context;
    std::unique_ptr<dc1394camera_list_t, Deleters::camera_list> cameraList;
};

class IIDCCamera: public Driver::Camera
{
    dc1394_t *context;
    dc1394camera_id_t camId;

    QString m_Name;
    QString vendor;

public:

    typedef std::shared_ptr<IIDCCamera> ptr;

    IIDCCamera(dc1394_t *_context, const dc1394camera_id_t &id, const QString &name, const QString &_vendor)
        : context(_context), camId(id), m_Name(name), vendor(_vendor) { }

    virtual ~IIDCCamera();

    Imager *imager(const ImageHandler::ptr &imageHandler) const override;

    QString name() const override { return m_Name; }
};

IIDCCamera::~IIDCCamera()
{
}

Imager *IIDCCamera::imager(const ImageHandler::ptr &imageHandler) const
{
    std::unique_ptr<dc1394camera_t, Deleters::camera> cam(dc1394_camera_new_unit(context, camId.guid, camId.unit));

    //TODO: what's the correct way of reporting fatal error from here?
    if (!cam)
    {
        const char *msg = "Could not initialize IIDC camera";
        qDebug() << msg;
        throw IIDCException(0, msg);
    }

    return new IIDCImager(std::move(cam), imageHandler, m_Name, vendor);
}

Driver::Cameras IIDCDriver::cameras() const
{
    if (!d->context)
        return { };

    Driver::Cameras cameras;

    dc1394camera_list_t *ptr;
    auto result = dc1394_camera_enumerate(d->context.get(), &ptr);
    if (DC1394_SUCCESS != result)
    {
        qDebug() << "Could not enumerate IIDC cameras: " << dc1394_error_get_string(result);
        return { };
    }

    d->cameraList.reset(ptr);

    for (auto i = 0; i < d->cameraList->num; i++)
    {
        std::unique_ptr<dc1394camera_t, Deleters::camera> cam(dc1394_camera_new_unit(d->context.get(),
                                                                                     d->cameraList->ids[i].guid,
                                                                                     d->cameraList->ids[i].unit));

        if (!cam)
        {
            qDebug() << "Could not initialize camera GUID " << d->cameraList->ids[i].guid
                     << "/unit " << d->cameraList->ids[i].unit;
        }
        else
        {
            // Appending driver name, because FLIR/Point Grey cameras can be also detected & enumerated via FlyCapture2 driver
            QString name = QString(cam->model) + " (IIDC)";

            cameras.push_back(std::make_shared<IIDCCamera>(d->context.get(),
                                                           d->cameraList->ids[i],
                                                           name, cam->vendor));
            qDebug() << "IIDC camera index " << i << " is " << name;
        }
    }

    return cameras;
}

static void IIDCLogHandler(dc1394log_t type, const char *msg, void*)
{
    switch (type)
    {
    case DC1394_LOG_DEBUG:   qDebug()    << "(IIDC) " << msg; break;
    case DC1394_LOG_WARNING: qWarning()  << "(IIDC) " << msg; break;
    case DC1394_LOG_ERROR:   qCritical() << "(IIDC) " << msg; break;
    }
}

IIDCDriver::IIDCDriver(): dptr()
{
    for (auto logType: { DC1394_LOG_ERROR, DC1394_LOG_WARNING, DC1394_LOG_DEBUG })
        dc1394_log_register_handler(logType, IIDCLogHandler, nullptr);

    d->context.reset(dc1394_new());
    if (!d->context)
        qDebug() << "Could not initialize IIDC";
}

IIDCDriver::~IIDCDriver() { }
