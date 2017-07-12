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

#include <cstring>
#include <QRect>
#include <FlyCapture2Defs.h>
#include <functional>
#include <map>
#include <tuple>
#include <vector>

#include "commons/utils.h"
#include "drivers/roi.h"
#include "fc2_exception.h"
#include "fc2_imager.h"
#include "fc2_worker.h"
#include "Qt/strings.h"


using namespace std::chrono;

enum ControlID: qlonglong
{
    VideoMode = FlyCapture2::PropertyType::UNSPECIFIED_PROPERTY_TYPE + 1,

    // Used to select one of the fixed frame rates from the 'FrameRate' enum (only for non-Format7 modes);
    // a camera may also support PropertyType::SHUTTER, which can change the frame rate independently and with finer granularity
    FrameRate,

    // Used to select pixel format for the current video mode (non-Format7 modes have only one pixel format)
    PixelFormat
};

static std::map<FlyCapture2::PixelFormat, const char *> PIXEL_FORMAT_NAME =
{
    { FlyCapture2::PIXEL_FORMAT_MONO8,    "Mono 8-bit"           },
    { FlyCapture2::PIXEL_FORMAT_411YUV8,  "YUV411"               },
    { FlyCapture2::PIXEL_FORMAT_422YUV8,  "YUV422"               },
    { FlyCapture2::PIXEL_FORMAT_444YUV8,  "YUV444"               },
    { FlyCapture2::PIXEL_FORMAT_RGB8,     "RGB 8-bit"            },
    { FlyCapture2::PIXEL_FORMAT_MONO16,   "Mono 16-bit"          },
    { FlyCapture2::PIXEL_FORMAT_RGB16,    "RGB 16-bit"           },
    { FlyCapture2::PIXEL_FORMAT_S_MONO16, "Mono 16-bit (signed)" },
    { FlyCapture2::PIXEL_FORMAT_S_RGB16,  "RGB 16-bit (signed) " },
    { FlyCapture2::PIXEL_FORMAT_RAW8,     "RAW 8-bit"            },
    { FlyCapture2::PIXEL_FORMAT_RAW16,    "RAW 16-bit"           },
    { FlyCapture2::PIXEL_FORMAT_MONO12,   "Mono 12-bit"          },
    { FlyCapture2::PIXEL_FORMAT_RAW12,    "RAW 12-bit"           },
    { FlyCapture2::PIXEL_FORMAT_BGR,      "BGR 8-bit"            },
    { FlyCapture2::PIXEL_FORMAT_BGRU,     "BGRU 8-bit"           },
    { FlyCapture2::PIXEL_FORMAT_RGB,      "RGB 8-bit"            },
    { FlyCapture2::PIXEL_FORMAT_RGBU,     "RGBU 8-bit"           },
    { FlyCapture2::PIXEL_FORMAT_BGR16,    "BGR 16-bit"           },
    { FlyCapture2::PIXEL_FORMAT_BGRU16,   "BGRU 16-bit"          },
    { FlyCapture2::PIXEL_FORMAT_422YUV8_JPEG, "JPEG"             }
};

static std::map<FlyCapture2::VideoMode, std::tuple<unsigned, unsigned>> VID_MODE_RESOLUTION =
{
    { FlyCapture2::VIDEOMODE_160x120YUV444   , { 160, 120 }   },
    { FlyCapture2::VIDEOMODE_320x240YUV422   , { 320, 240 }   },
    { FlyCapture2::VIDEOMODE_640x480YUV411   , { 640, 480 }   },
    { FlyCapture2::VIDEOMODE_640x480YUV422   , { 640, 480 }   },
    { FlyCapture2::VIDEOMODE_640x480RGB      , { 640, 480 }   },
    { FlyCapture2::VIDEOMODE_640x480Y8       , { 640, 480 }   },
    { FlyCapture2::VIDEOMODE_640x480Y16      , { 640, 480 }   },
    { FlyCapture2::VIDEOMODE_800x600YUV422   , { 800, 600 }   },
    { FlyCapture2::VIDEOMODE_800x600RGB      , { 800, 600 }   },
    { FlyCapture2::VIDEOMODE_800x600Y8       , { 800, 600 }   },
    { FlyCapture2::VIDEOMODE_800x600Y16      , { 800, 600 }   },
    { FlyCapture2::VIDEOMODE_1024x768YUV422  , { 1024, 768 }  },
    { FlyCapture2::VIDEOMODE_1024x768RGB     , { 1024, 768 }  },
    { FlyCapture2::VIDEOMODE_1024x768Y8      , { 1024, 768 }  },
    { FlyCapture2::VIDEOMODE_1024x768Y16     , { 1024, 768 }  },
    { FlyCapture2::VIDEOMODE_1280x960YUV422  , { 1280, 960 }  },
    { FlyCapture2::VIDEOMODE_1280x960RGB     , { 1280, 960 }  },
    { FlyCapture2::VIDEOMODE_1280x960Y8      , { 1280, 960 }  },
    { FlyCapture2::VIDEOMODE_1280x960Y16     , { 1280, 960 }  },
    { FlyCapture2::VIDEOMODE_1600x1200YUV422 , { 1600, 1200 } },
    { FlyCapture2::VIDEOMODE_1600x1200RGB    , { 1600, 1200 } },
    { FlyCapture2::VIDEOMODE_1600x1200Y8     , { 1600, 1200 } },
    { FlyCapture2::VIDEOMODE_1600x1200Y16    , { 1600, 1200 } },
};

static std::map<FlyCapture2::VideoMode, FlyCapture2::PixelFormat> VID_MODE_PIX_FMT =
{
    { FlyCapture2::VIDEOMODE_160x120YUV444   , FlyCapture2::PIXEL_FORMAT_444YUV8 },
    { FlyCapture2::VIDEOMODE_320x240YUV422   , FlyCapture2::PIXEL_FORMAT_422YUV8 },
    { FlyCapture2::VIDEOMODE_640x480YUV411   , FlyCapture2::PIXEL_FORMAT_411YUV8 },
    { FlyCapture2::VIDEOMODE_640x480YUV422   , FlyCapture2::PIXEL_FORMAT_422YUV8 },
    { FlyCapture2::VIDEOMODE_640x480RGB      , FlyCapture2::PIXEL_FORMAT_RGB     },
    { FlyCapture2::VIDEOMODE_640x480Y8       , FlyCapture2::PIXEL_FORMAT_MONO8   },
    { FlyCapture2::VIDEOMODE_640x480Y16      , FlyCapture2::PIXEL_FORMAT_MONO16  },
    { FlyCapture2::VIDEOMODE_800x600YUV422   , FlyCapture2::PIXEL_FORMAT_422YUV8 },
    { FlyCapture2::VIDEOMODE_800x600RGB      , FlyCapture2::PIXEL_FORMAT_RGB8    },
    { FlyCapture2::VIDEOMODE_800x600Y8       , FlyCapture2::PIXEL_FORMAT_MONO8   },
    { FlyCapture2::VIDEOMODE_800x600Y16      , FlyCapture2::PIXEL_FORMAT_MONO16  },
    { FlyCapture2::VIDEOMODE_1024x768YUV422  , FlyCapture2::PIXEL_FORMAT_422YUV8 },
    { FlyCapture2::VIDEOMODE_1024x768RGB     , FlyCapture2::PIXEL_FORMAT_RGB8    },
    { FlyCapture2::VIDEOMODE_1024x768Y8      , FlyCapture2::PIXEL_FORMAT_MONO8   },
    { FlyCapture2::VIDEOMODE_1024x768Y16     , FlyCapture2::PIXEL_FORMAT_MONO16  },
    { FlyCapture2::VIDEOMODE_1280x960YUV422  , FlyCapture2::PIXEL_FORMAT_422YUV8 },
    { FlyCapture2::VIDEOMODE_1280x960RGB     , FlyCapture2::PIXEL_FORMAT_RGB8    },
    { FlyCapture2::VIDEOMODE_1280x960Y8      , FlyCapture2::PIXEL_FORMAT_MONO8   },
    { FlyCapture2::VIDEOMODE_1280x960Y16     , FlyCapture2::PIXEL_FORMAT_MONO16  },
    { FlyCapture2::VIDEOMODE_1600x1200YUV422 , FlyCapture2::PIXEL_FORMAT_422YUV8 },
    { FlyCapture2::VIDEOMODE_1600x1200RGB    , FlyCapture2::PIXEL_FORMAT_RGB8    },
    { FlyCapture2::VIDEOMODE_1600x1200Y8     , FlyCapture2::PIXEL_FORMAT_MONO8   },
    { FlyCapture2::VIDEOMODE_1600x1200Y16    , FlyCapture2::PIXEL_FORMAT_MONO16  },
};

/// Key: FlyCapture2::PropertyInfo::pUnitAbbr
static const std::map<std::string, std::chrono::duration<double>> DURATION_VALUE =
{
    { "us", 1us },
    { "ms", 1ms },
    { "s",  1s }
};

DPTR_IMPL(FC2Imager)
{
    FlyCapture2::Camera cam;
    FlyCapture2::CameraInfo camInfo;

    std::map<FlyCapture2::VideoMode, std::vector<FlyCapture2::FrameRate>> videoModes;
    std::map<FlyCapture2::Mode, FlyCapture2::Format7Info> fmt7Modes;
    std::map<FlyCapture2::PropertyType, FlyCapture2::PropertyInfo> propertyInfo;

    FC2VideoMode currentVidMode;
    FlyCapture2::FrameRate currentFrameRate;
    FlyCapture2::PixelFormat currentPixFmt;

    /// Copy of the SHUTTER control; used for informing GUI about shutter range change when frame rate changes
    Control ctrlShutter;

    ROIValidator::ptr roiValidator; ///< Region of Interest validator

    /// Concerns the current video mode; used to disable ROI and return to full image size
    QSize maxFrameSize;

    QRect currentROI;

    bool temperatureAvailable;

    void updateWorkerExposureTimeout();

    Control enumerateVideoModes();

    /// Returns a combo control listing all pixel formats for 'currentVidMode'
    Control enumerateCurrentModePixelFormats();

    /** Resets frame size, ROI validator and frame rate (if applicable). The actual video mode change
        comes into effect after the worker thread is restarted. */
    void changeVideoMode(const FC2VideoMode &newMode);

    Control getFrameRates(FC2VideoMode vidMode);

    void updateShutterCtrl();

    Control getEmptyFrameRatesCtrl();


    LOG_C_SCOPE(FC2Imager);
};

/// Ensures both raw and absolute ranges satisfy: min < max
static void VerifyRanges(FlyCapture2::PropertyInfo &propInfo)
{
    if (propInfo.min > propInfo.max)
        std::swap(propInfo.min, propInfo.max);

    if (propInfo.absMin > propInfo.absMax)
        std::swap(propInfo.absMin, propInfo.absMax);
}

static void UpdateRangeAndStep(Imager::Control &control, const FlyCapture2::PropertyInfo &propInfo);

void FC2Imager::Private::updateWorkerExposureTimeout()
{
//TODO: implement this
}

static void ForEachPossiblePixelFormat(const std::function<void (FlyCapture2::PixelFormat)> &func)
{
    // TODO: implement handling of the commented out formats
    for (const auto pixFmt: {
            FlyCapture2::PIXEL_FORMAT_MONO8,
            FlyCapture2::PIXEL_FORMAT_RAW8,
            FlyCapture2::PIXEL_FORMAT_RGB8,

            //TODO: handle this
            // FlyCapture2::PIXEL_FORMAT_411YUV8,
            // FlyCapture2::PIXEL_FORMAT_422YUV8,
            // FlyCapture2::PIXEL_FORMAT_444YUV8,

            FlyCapture2::PIXEL_FORMAT_MONO16,
            FlyCapture2::PIXEL_FORMAT_RAW16,
            FlyCapture2::PIXEL_FORMAT_RGB16,

            //TODO: handle this
            // FlyCapture2::PIXEL_FORMAT_MONO12,
            // FlyCapture2::PIXEL_FORMAT_RAW12,
            /* FlyCapture2::PIXEL_FORMAT_422YUV8_JPEG*/ })
    {
        func(pixFmt);
    }
}

Imager::Control FC2Imager::Private::enumerateCurrentModePixelFormats()
{
    auto pixelFormats = Imager::Control{ ControlID::PixelFormat, "Pixel Format", Control::Combo };

    if (currentVidMode.isFormat7())
    {
        const auto &f7info = fmt7Modes[(FlyCapture2::Mode)currentVidMode];

        ForEachPossiblePixelFormat(
            [&f7info, &pixelFormats](FlyCapture2::PixelFormat pixFmt)
            {
                if ((f7info.pixelFormatBitField       & pixFmt) == pixFmt ||
                    (f7info.vendorPixelFormatBitField & pixFmt) == pixFmt)
                {
                    pixelFormats.add_choice_enum(PIXEL_FORMAT_NAME[pixFmt], pixFmt);
                }
            });
    }
    else
        pixelFormats.add_choice_enum(PIXEL_FORMAT_NAME[currentPixFmt], currentPixFmt);

    pixelFormats.set_value_enum(currentPixFmt);
    pixelFormats.readonly = (pixelFormats.choices.size() == 1);

    return pixelFormats;
}

Imager::Control FC2Imager::Private::enumerateVideoModes()
{
    auto videoMode = Imager::Control{ ControlID::VideoMode, "Video Mode", Control::Combo };

    for (const auto &vm: videoModes)
    {
        const FlyCapture2::VideoMode vidMode = vm.first;
        auto resolution = VID_MODE_RESOLUTION[vidMode];
        QString modeName = "%1x%2 %3"_q % (int)std::get<0>(resolution) % (int)std::get<1>(resolution) % PIXEL_FORMAT_NAME[VID_MODE_PIX_FMT[vidMode]];
        videoMode.add_choice_enum(modeName, (FC2VideoMode)vidMode);
    }

    for (const auto &f7m: fmt7Modes)
    {
        QString modeName = "%1x%2 (FMT7: %4)"_q % f7m.second.maxWidth
                                                % f7m.second.maxHeight
                                                % (f7m.second.mode - FlyCapture2::MODE_0);

        videoMode.add_choice_enum(modeName, (FC2VideoMode)f7m.first);
    }

    videoMode.set_value_enum((int)currentVidMode);

    return videoMode;
}

Imager::Control FC2Imager::Private::getEmptyFrameRatesCtrl()
{
    auto frameRatesCtrl = Imager::Control{ ControlID::FrameRate, "Fixed Frame Rate", Control::Combo };

    frameRatesCtrl.add_choice_enum("N/A", 0);
    frameRatesCtrl.set_value_enum(0);
    frameRatesCtrl.readonly = true;

    return frameRatesCtrl;
}

Imager::Control FC2Imager::Private::getFrameRates(FC2VideoMode vidMode)
{
    auto frameRatesCtrl = Imager::Control{ ControlID::FrameRate, "Fixed Frame Rate", Control::Combo };

    if (!vidMode.isFormat7())
    {
        const auto frloc = videoModes.find((FlyCapture2::VideoMode)vidMode);
        assert(frloc != videoModes.end());

        const auto &frList = frloc->second;

        for (const auto fr: frList)
            switch (fr)
            {
            case FlyCapture2::FRAMERATE_1_875: frameRatesCtrl.add_choice_enum("1.875 fps", fr); break;
            case FlyCapture2::FRAMERATE_3_75:  frameRatesCtrl.add_choice_enum("3.75 fps",  fr); break;
            case FlyCapture2::FRAMERATE_7_5:   frameRatesCtrl.add_choice_enum("7.5 fps",   fr); break;
            case FlyCapture2::FRAMERATE_15:    frameRatesCtrl.add_choice_enum("15 fps",    fr); break;
            case FlyCapture2::FRAMERATE_30:    frameRatesCtrl.add_choice_enum("30 fps",    fr); break;
            case FlyCapture2::FRAMERATE_60:    frameRatesCtrl.add_choice_enum("60 fps",    fr); break;
            case FlyCapture2::FRAMERATE_120:   frameRatesCtrl.add_choice_enum("120 fps",   fr); break;
            case FlyCapture2::FRAMERATE_240:   frameRatesCtrl.add_choice_enum("240 fps",   fr); break;
            }

        // Initially we set the highest frame rate, so report it as selected
        frameRatesCtrl.set_value_enum(*std::max_element(frList.begin(), frList.end()));

        return frameRatesCtrl;
    }
    else
        return getEmptyFrameRatesCtrl();
}

static FlyCapture2::PixelFormat GetFirstSupportedPixelFormat(const FlyCapture2::Format7Info &f7info)
{
    FlyCapture2::PixelFormat first = FlyCapture2::UNSPECIFIED_PIXEL_FORMAT;
    bool firstAlreadySelected = false;

    ForEachPossiblePixelFormat([&](FlyCapture2::PixelFormat pixFmt)
                               {
                                   if ((f7info.pixelFormatBitField & pixFmt) == pixFmt && !firstAlreadySelected)
                                   {
                                       first = pixFmt;
                                       firstAlreadySelected = true;
                                   }
                               });

    return first;
}

FC2Imager::FC2Imager(const FlyCapture2::PGRGuid &guid, const ImageHandler::ptr &handler)
: Imager(handler), dptr()
{
    //FIXME: if a CHECK fails in Imager constructor, there is a segfault (instead of printing the caught exception)

    d->temperatureAvailable = false;

    // Connect() most likely does not modify the passed GUID
    FC2_CHECK << d->cam.Connect(const_cast<FlyCapture2::PGRGuid*>(&guid)).GetType()
              << "Camera::Connect";

    FC2_CHECK << d->cam.GetCameraInfo(&d->camInfo).GetType()
              << "Camera::GetCameraInfo";

    for (int vidMode = 0; vidMode < FlyCapture2::VIDEOMODE_FORMAT7; vidMode++)
        for (int frameRate = 0; frameRate < FlyCapture2::FRAMERATE_FORMAT7; frameRate++)
        {
            bool supported = false;

            FC2_CHECK << d->cam.GetVideoModeAndFrameRateInfo((FlyCapture2::VideoMode)vidMode, (FlyCapture2::FrameRate)frameRate, &supported).GetType()
                      << "Camera::GetVideoModeAndFrameRateInfo";

            if (supported)
                d->videoModes[(FlyCapture2::VideoMode)vidMode].push_back((FlyCapture2::FrameRate)frameRate);
        }

    for (int fmt7Mode = 0; fmt7Mode <= FlyCapture2::MODE_31; fmt7Mode++)
    {
        FlyCapture2::Format7Info f7info;
        f7info.mode = (FlyCapture2::Mode)fmt7Mode;
        bool supported = false;

        FC2_CHECK << d->cam.GetFormat7Info(&f7info, &supported).GetType()
                  << "Camera::GetFormat7Info";

        if (supported)
            d->fmt7Modes[(FlyCapture2::Mode)fmt7Mode] = f7info;
    }

    if (!d->videoModes.empty())
        d->changeVideoMode(d->videoModes.begin()->first);
    else
        d->changeVideoMode(d->fmt7Modes.begin()->first);

    if (d->currentVidMode.isFormat7())
    {
        const auto &f7info = d->fmt7Modes[(FlyCapture2::Mode)d->currentVidMode];
        d->currentPixFmt = GetFirstSupportedPixelFormat(f7info);
    }
    else
        d->currentPixFmt = VID_MODE_PIX_FMT[(FlyCapture2::VideoMode)d->currentVidMode];

    connect(this, &Imager::exposure_changed, this, std::bind(&Private::updateWorkerExposureTimeout, d.get()));
}

FC2Imager::~FC2Imager()
{
}

Imager::Properties FC2Imager::properties() const
{
    auto properties = Imager::Properties();
    properties << LiveStream;

    if (!d->fmt7Modes.empty())
        properties << ROI;

    properties << Imager::Properties::Property{ "Vendor", d->camInfo.vendorName };

    QString interfaceStr;
    switch (d->camInfo.interfaceType)
    {
    case FlyCapture2::INTERFACE_GIGE: interfaceStr = "Gigabit Ethernet"; break;
    case FlyCapture2::INTERFACE_IEEE1394: interfaceStr = "IEEE 1394"; break;
    case FlyCapture2::INTERFACE_USB2: interfaceStr = "USB 2.0"; break;
    case FlyCapture2::INTERFACE_USB3: interfaceStr = "USB 3.0"; break;
    default: interfaceStr = "unknown"; break;
    }
    properties << Imager::Properties::Property{ "Interface", interfaceStr };

    QString driverStr;
    switch (d->camInfo.driverType)
    {
    case FlyCapture2::DRIVER_1394_CAM:       driverStr = "PGRCam.sys"; break;
    case FlyCapture2::DRIVER_1394_PRO:       driverStr = "PGR1394.sys"; break;
    case FlyCapture2::DRIVER_1394_JUJU:      driverStr = "firewire_core"; break;
    case FlyCapture2::DRIVER_1394_VIDEO1394: driverStr = "video1394"; break;
    case FlyCapture2::DRIVER_1394_RAW1394:   driverStr = "raw1394"; break;
    case FlyCapture2::DRIVER_USB_NONE:       driverStr = "native"; break;
    case FlyCapture2::DRIVER_USB_CAM:        driverStr = "PGRUsbCam.sys"; break;
    case FlyCapture2::DRIVER_USB3_PRO:       driverStr = "PGRXHCI.sys"; break;
    case FlyCapture2::DRIVER_GIGE_NONE:      driverStr = "native"; break;
    case FlyCapture2::DRIVER_GIGE_FILTER:    driverStr = "PGRGigE.sys"; break;
    case FlyCapture2::DRIVER_GIGE_PRO:       driverStr = "PGRGigEPro.sys"; break;
    case FlyCapture2::DRIVER_GIGE_LWF:       driverStr = "PgrLwf.sys"; break;
    default: driverStr = "unknown"; break;
    }
    properties << Imager::Properties::Property{ "Driver", driverStr };

    properties << Imager::Properties::Property{ "Sensor", d->camInfo.sensorInfo };

    properties << Imager::Properties::Property{ "Resolution", d->camInfo.sensorResolution };

    properties << Imager::Properties::Property{ "Firmware Version", d->camInfo.firmwareVersion };

    properties << Imager::Properties::Property{ "Firmware Build Time", d->camInfo.firmwareBuildTime };

    if (d->camInfo.maximumBusSpeed != FlyCapture2::BUSSPEED_SPEED_UNKNOWN)
    {
        QString maxBusSpeedStr;
        switch (d->camInfo.maximumBusSpeed)
        {
        case FlyCapture2::BUSSPEED_S100:        maxBusSpeedStr = "100 Mb/s"; break;
        case FlyCapture2::BUSSPEED_S200:        maxBusSpeedStr = "200 Mb/s"; break;
        case FlyCapture2::BUSSPEED_S400:        maxBusSpeedStr = "400 Mb/s"; break;
        case FlyCapture2::BUSSPEED_S480:        maxBusSpeedStr = "480 Mb/s"; break;
        case FlyCapture2::BUSSPEED_S800:        maxBusSpeedStr = "800 Mb/s"; break;
        case FlyCapture2::BUSSPEED_S1600:       maxBusSpeedStr = "1.6 Gb/s"; break;
        case FlyCapture2::BUSSPEED_S3200:       maxBusSpeedStr = "3.2 Gb/s"; break;
        case FlyCapture2::BUSSPEED_S5000:       maxBusSpeedStr = "5.0 Gb/s"; break;
        case FlyCapture2::BUSSPEED_10BASE_T:    maxBusSpeedStr = "10Base-T"; break;
        case FlyCapture2::BUSSPEED_100BASE_T:   maxBusSpeedStr = "100Base-T"; break;
        case FlyCapture2::BUSSPEED_1000BASE_T:  maxBusSpeedStr = "1000Base-T"; break;
        case FlyCapture2::BUSSPEED_10000BASE_T: maxBusSpeedStr = "10000Base-T"; break;
        case FlyCapture2::BUSSPEED_S_FASTEST:   maxBusSpeedStr = "Fastest available"; break;
        case FlyCapture2::BUSSPEED_ANY:         maxBusSpeedStr = "Any available"; break;
        }

        properties << Imager::Properties::Property{ "Max. Bus Speed", maxBusSpeedStr };
    }

    if (d->camInfo.interfaceType == FlyCapture2::INTERFACE_GIGE)
    {
        QString macStr;
        macStr.sprintf("%02X:%02X:%02X:%02X:%02X:%02X", d->camInfo.macAddress.octets[0],
                                                        d->camInfo.macAddress.octets[1],
                                                        d->camInfo.macAddress.octets[2],
                                                        d->camInfo.macAddress.octets[3],
                                                        d->camInfo.macAddress.octets[4],
                                                        d->camInfo.macAddress.octets[5]);

        properties << Imager::Properties::Property{ "MAC",  macStr };

        auto ipFormatter = [](const FlyCapture2::IPAddress &ip)
                           {
                               QString str;
                               str.sprintf("%d.%d.%d.%d", ip.octets[0], ip.octets[1], ip.octets[2], ip.octets[3]);
                               return str;
                           };

        properties << Imager::Properties::Property{ "IP Address", ipFormatter(d->camInfo.ipAddress) };

        properties << Imager::Properties::Property{ "Subnet Mask", ipFormatter(d->camInfo.subnetMask) };
    }

    FlyCapture2::PropertyInfo propInfo;
    propInfo.type = FlyCapture2::TEMPERATURE;
    FC2_CHECK << d->cam.GetPropertyInfo(&propInfo).GetType()
              << "Camera::GetPropertyInfo";
    if (propInfo.present)
    {
        d->temperatureAvailable = true;
        properties << Temperature;
    }


    return properties;
}

QString FC2Imager::name() const
{
    return d->camInfo.modelName;
}

void FC2Imager::Private::changeVideoMode(const FC2VideoMode &newMode)
{
    if (newMode.isFormat7())
    {
        const auto &fmt7 = fmt7Modes[(FlyCapture2::Mode)newMode];

        roiValidator = std::make_shared<ROIValidator>(
            std::list<ROIValidator::Rule>{ ROIValidator::x_multiple(fmt7.offsetHStepSize),
                                           ROIValidator::y_multiple(fmt7.offsetVStepSize),
                                           ROIValidator::width_multiple(fmt7.imageHStepSize),
                                           ROIValidator::height_multiple(fmt7.imageVStepSize),
                                           ROIValidator::within_rect(QRect{ 0, 0, (int)fmt7.maxWidth, (int)fmt7.maxHeight }) });

        maxFrameSize = { (int)fmt7.maxWidth, (int)fmt7.maxHeight };

        currentROI = { 0, 0, maxFrameSize.width(), maxFrameSize.height() };

        currentPixFmt = GetFirstSupportedPixelFormat(fmt7);
    }
    else
    {
        const auto &res = VID_MODE_RESOLUTION[(FlyCapture2::VideoMode)newMode];
        const int width  = std::get<0>(res);
        const int height = std::get<1>(res);

        currentROI = { 0, 0, width, height };

        roiValidator = std::make_shared<ROIValidator>(std::list<ROIValidator::Rule>{ });

        // Initially choose the highest frame rate
        currentFrameRate = *videoModes[(FlyCapture2::VideoMode)newMode].rbegin();
    }

    currentVidMode = newMode;
}

void FC2Imager::setControl(const Imager::Control& control)
{
    LOG_F_SCOPE

    if (control.id == ControlID::VideoMode)
    {
        const auto newMode = control.get_value_enum<FC2VideoMode>();

        d->changeVideoMode(newMode);

        startLive();

        if (newMode.isFormat7())
        {
            // Format7 modes do not use fixed frame rates; inform the GUI to show an empty list
            emit changed(d->getEmptyFrameRatesCtrl());
        }
        else
            emit changed(d->getFrameRates(newMode));

        emit changed(d->enumerateCurrentModePixelFormats());
    }
    else if (control.id == ControlID::FrameRate)
    {
        d->currentFrameRate = control.get_value_enum<FlyCapture2::FrameRate>();
        startLive();
    }
    else if (control.id == ControlID::PixelFormat)
    {
        d->currentPixFmt = control.get_value_enum<FlyCapture2::PixelFormat>();
        startLive();
    }
    else
    {
        FlyCapture2::Property prop;
        prop.type = (FlyCapture2::PropertyType)control.id;
        FC2_CHECK << d->cam.GetProperty(&prop).GetType()
                  << "Camera::GetProperty";

        prop.autoManualMode = control.value_auto;
        prop.onOff = control.value_onOff;
        prop.absValue = control.value.toFloat();
        prop.valueA = control.value.toInt();

        FC2_CHECK << d->cam.SetProperty(&prop).GetType()
                  << "Camera::SetProperty";
    }

    emit changed(control);

    if (d->ctrlShutter.valid() &&
        (FlyCapture2::FRAME_RATE == control.id ||
         ControlID::FrameRate    == control.id ||
         ControlID::VideoMode    == control.id ||
         ControlID::PixelFormat  == control.id))
    {
        // Changing frame rate, video mode or pixel format may change the shutter range; need to inform the GUI

        d->updateShutterCtrl();
        emit changed(d->ctrlShutter);
    }
}

void FC2Imager::readTemperature()
{
    if (d->temperatureAvailable)
    {
        FlyCapture2::Property prop;
        prop.type = FlyCapture2::TEMPERATURE;
        FC2_CHECK << d->cam.GetProperty(&prop).GetType()
                  << "Camera::GetProperty";

        // Calculation as in CamSettingsPage.cpp from FlyCapture2 SDK. Strangely, both "absolute capable"
        // and "unit abbreviation" fields are not taken into account (e.g. on Chameleon3 CM3-U3-13S2M the indicated
        // unit is Celsius; yet, the formula below must be used anyway).

        double tempCelsius = prop.valueA / 10.0 - 273.15;

        emit temperature(tempCelsius);
    }
}

static void UpdateRangeAndStep(Imager::Control &control, const FlyCapture2::PropertyInfo &propInfo)
{
    if (propInfo.absValSupported)
    {
        if (propInfo.max != propInfo.min)
            control.range.step = (propInfo.absMax - propInfo.absMin) / (propInfo.max - propInfo.min + 1);
        else
            control.range.step = 0;

        control.range.min = propInfo.absMin;
        control.range.max = propInfo.absMax;
    }
    else
    {
        control.range.min = propInfo.min;
        control.range.max = propInfo.max;
        control.range.step = (propInfo.min != propInfo.max ? 1 : 0);
    }
}

Imager::Controls FC2Imager::controls() const
{
    Controls controls;

    controls.push_back(std::move(d->enumerateVideoModes()));

    controls.push_back(std::move(d->enumerateCurrentModePixelFormats()));

    controls.push_back(d->getFrameRates(d->currentVidMode));

    for (FlyCapture2::PropertyType propType: { FlyCapture2::BRIGHTNESS,
                                               FlyCapture2::AUTO_EXPOSURE,
                                               FlyCapture2::SHARPNESS,
                                               FlyCapture2::WHITE_BALANCE,
                                               FlyCapture2::HUE,
                                               FlyCapture2::SATURATION,
                                               FlyCapture2::GAMMA,
                                               FlyCapture2::IRIS,
                                               FlyCapture2::FOCUS,
                                               FlyCapture2::ZOOM,
                                               FlyCapture2::PAN,
                                               FlyCapture2::TILT,
                                               FlyCapture2::SHUTTER,
                                               FlyCapture2::GAIN,
                                               FlyCapture2::TRIGGER_MODE,
                                               FlyCapture2::TRIGGER_DELAY,
                                               FlyCapture2::FRAME_RATE })
    {
        FlyCapture2::PropertyInfo propInfo;
        propInfo.type = propType;

        FC2_CHECK << d->cam.GetPropertyInfo(&propInfo).GetType()
                  << "Camera::GetPropertyInfo";

        VerifyRanges(propInfo);

        if (propInfo.present)
        {
            d->propertyInfo[propType] = propInfo;

            Control control{ propType };
            control.type = Control::Type::Number;

            FlyCapture2::Property prop;
            prop.type = propType;
            FC2_CHECK << d->cam.GetProperty(&prop).GetType()
                      << "Camera::GetProperty";

            switch (propType)
            {
                case FlyCapture2::BRIGHTNESS:       control.name = "Brightness"; break;

                // This is not "exposure time" (see FlyCapture2::SHUTTER for that);
                // instead, it regulates the desired overall image brightness by changing
                // values of shutter and gain - if they are set to auto
                case FlyCapture2::AUTO_EXPOSURE:    control.name = "Exposure"; break;

                case FlyCapture2::SHARPNESS:       control.name = "Sharpness"; break;

                case FlyCapture2::HUE:             control.name = "Hue"; break;
                case FlyCapture2::SATURATION:      control.name = "Saturation"; break;
                case FlyCapture2::GAMMA:           control.name = "Gamma"; break;

                case FlyCapture2::SHUTTER:
                {
                    control.name = "Shutter";
                    control.is_exposure = true;
                    control.is_duration = true;

                    const auto &unitLoc = DURATION_VALUE.find(propInfo.pUnitAbbr);
                    if (unitLoc != DURATION_VALUE.end())
                        control.duration_unit = unitLoc->second;
                    else
                        control.duration_unit = 1s;
                    break;
                }

                case FlyCapture2::GAIN:            control.name = "Gain"; break;
                case FlyCapture2::IRIS:            control.name = "Iris"; break;
                case FlyCapture2::FOCUS:           control.name = "Focus"; break;
                //TODO: implement this      case FlyCapture2::WHITE_BALANCE:   control.name = "White Balance"; break;
                case FlyCapture2::FRAME_RATE:      control.name = "Frame Rate"; break;
                case FlyCapture2::ZOOM:            control.name = "Zoom"; break;
                case FlyCapture2::PAN:             control.name = "Pan"; break;
                case FlyCapture2::TILT:            control.name = "Tilt"; break;

                default: continue;
            }

            control.supports_auto = propInfo.autoSupported;

            control.readonly = !propInfo.manualSupported && !propInfo.autoSupported && propInfo.readOutSupported;

            control.supports_onOff = propInfo.onOffSupported;

            if (control.supports_onOff)
                control.value_onOff = prop.onOff;

            control.value_auto = prop.autoManualMode;

            // A feature is "absolute control-capable", if its value can be set using
            // floating-point arguments, not just the integer "raw/driver" values.
            // E.g. SHUTTER can be set in fractional "absolute" values expressed in seconds.
            if (propInfo.absValSupported)
            {
                control.decimals = 6;

                prop.absControl = true;
                FC2_CHECK << d->cam.SetProperty(&prop).GetType()
                          << "Camera::SetProperty - enable absolute control";

                //if (DC1394_TRUE == feature.readout_capable)
                if (propInfo.readOutSupported)
                {
                    FC2_CHECK << d->cam.GetProperty(&prop).GetType()
                              << "Camera::GetProperty";

                    control.value = prop.absValue;
                }
                else
                    control.value = propInfo.absMin;
            }
            else
            {
                control.decimals = 0;

                if (propInfo.readOutSupported)
                    control.value = prop.valueA;
                else
                    control.value = propInfo.min;
            }

            UpdateRangeAndStep(control, propInfo);

            if (propType == FlyCapture2::SHUTTER)
                d->ctrlShutter = control; // See the comment for ctrlShutter

            controls.push_back(std::move(control));
        }
    }

    return controls;
}


void FC2Imager::clearROI()
{
    setROI(QRect{ 0, 0, d->maxFrameSize.width(), d->maxFrameSize.height() });
}

void FC2Imager::setROI(const QRect &roi)
{
    d->currentROI = d->roiValidator->validate(roi, QRect{ });
    startLive();
}


void FC2Imager::startLive()
{
    restart([this] { return std::make_shared<FC2ImagerWorker>(d->cam, d->currentVidMode, d->currentFrameRate, d->currentPixFmt, d->currentROI); });
    qDebug() << "Video streaming started successfully";
}

void FC2Imager::Private::updateShutterCtrl()
{
    FlyCapture2::PropertyInfo propInfo;
    propInfo.type = FlyCapture2::SHUTTER;

    FC2_CHECK << cam.GetPropertyInfo(&propInfo).GetType()
              << "Camera::GetPropertyInfo - shutter";

    VerifyRanges(propInfo);

    UpdateRangeAndStep(ctrlShutter, propInfo);
}
