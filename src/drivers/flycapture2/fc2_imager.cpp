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

#include <QRect>
#include <C/FlyCapture2_C.h>
#include <functional>
#include <map>
#include <tuple>
#include <vector>

#include "commons/utils.h"
#include "drivers/roi.h"
#include "fc2_exception.h"
#include "fc2_imager.h"
#include "fc2_worker.h"
#include "Qt/qt_strings_helper.h"


using namespace std::chrono;

enum ControlID: qlonglong
{
    VideoMode = FC2_UNSPECIFIED_PROPERTY_TYPE + 1,

    // Used to select one of the fixed frame rates from the 'fc2FrameRate' enum (only for non-Format7 modes);
    // a camera may also support FC2_FRAME_RATE, which can change the frame rate independently and with finer granularity
    FrameRate,

    // Used to select pixel format for the current video mode (non-Format7 modes have only one pixel format)
    PixelFormat,

    // FC2_WHITE_BALANCE is actually a pair of values, so expose two custom IDs
    WhiteBalanceRed, WhiteBalanceBlue
};

static std::map<fc2PixelFormat, const char *> PIXEL_FORMAT_NAME =
{
    { FC2_PIXEL_FORMAT_MONO8,    "Mono 8-bit"           },
    { FC2_PIXEL_FORMAT_411YUV8,  "YUV411"               },
    { FC2_PIXEL_FORMAT_422YUV8,  "YUV422"               },
    { FC2_PIXEL_FORMAT_444YUV8,  "YUV444"               },
    { FC2_PIXEL_FORMAT_RGB8,     "RGB 8-bit"            },
    { FC2_PIXEL_FORMAT_MONO16,   "Mono 16-bit"          },
    { FC2_PIXEL_FORMAT_RGB16,    "RGB 16-bit"           },
    { FC2_PIXEL_FORMAT_S_MONO16, "Mono 16-bit (signed)" },
    { FC2_PIXEL_FORMAT_S_RGB16,  "RGB 16-bit (signed) " },
    { FC2_PIXEL_FORMAT_RAW8,     "RAW 8-bit"            },
    { FC2_PIXEL_FORMAT_RAW16,    "RAW 16-bit"           },
    { FC2_PIXEL_FORMAT_MONO12,   "Mono 12-bit"          },
    { FC2_PIXEL_FORMAT_RAW12,    "RAW 12-bit"           },
    { FC2_PIXEL_FORMAT_BGR,      "BGR 8-bit"            },
    { FC2_PIXEL_FORMAT_BGRU,     "BGRU 8-bit"           },
    { FC2_PIXEL_FORMAT_RGB,      "RGB 8-bit"            },
    { FC2_PIXEL_FORMAT_RGBU,     "RGBU 8-bit"           },
    { FC2_PIXEL_FORMAT_BGR16,    "BGR 16-bit"           },
    { FC2_PIXEL_FORMAT_BGRU16,   "BGRU 16-bit"          },
    { FC2_PIXEL_FORMAT_422YUV8_JPEG, "JPEG"             }
};

static std::map<fc2VideoMode, std::tuple<unsigned, unsigned>> VID_MODE_RESOLUTION =
{
    { FC2_VIDEOMODE_160x120YUV444   , { 160, 120 }   },
    { FC2_VIDEOMODE_320x240YUV422   , { 320, 240 }   },
    { FC2_VIDEOMODE_640x480YUV411   , { 640, 480 }   },
    { FC2_VIDEOMODE_640x480YUV422   , { 640, 480 }   },
    { FC2_VIDEOMODE_640x480RGB      , { 640, 480 }   },
    { FC2_VIDEOMODE_640x480Y8       , { 640, 480 }   },
    { FC2_VIDEOMODE_640x480Y16      , { 640, 480 }   },
    { FC2_VIDEOMODE_800x600YUV422   , { 800, 600 }   },
    { FC2_VIDEOMODE_800x600RGB      , { 800, 600 }   },
    { FC2_VIDEOMODE_800x600Y8       , { 800, 600 }   },
    { FC2_VIDEOMODE_800x600Y16      , { 800, 600 }   },
    { FC2_VIDEOMODE_1024x768YUV422  , { 1024, 768 }  },
    { FC2_VIDEOMODE_1024x768RGB     , { 1024, 768 }  },
    { FC2_VIDEOMODE_1024x768Y8      , { 1024, 768 }  },
    { FC2_VIDEOMODE_1024x768Y16     , { 1024, 768 }  },
    { FC2_VIDEOMODE_1280x960YUV422  , { 1280, 960 }  },
    { FC2_VIDEOMODE_1280x960RGB     , { 1280, 960 }  },
    { FC2_VIDEOMODE_1280x960Y8      , { 1280, 960 }  },
    { FC2_VIDEOMODE_1280x960Y16     , { 1280, 960 }  },
    { FC2_VIDEOMODE_1600x1200YUV422 , { 1600, 1200 } },
    { FC2_VIDEOMODE_1600x1200RGB    , { 1600, 1200 } },
    { FC2_VIDEOMODE_1600x1200Y8     , { 1600, 1200 } },
    { FC2_VIDEOMODE_1600x1200Y16    , { 1600, 1200 } },
};

static std::map<fc2VideoMode, fc2PixelFormat> VID_MODE_PIX_FMT =
{
    { FC2_VIDEOMODE_160x120YUV444   , FC2_PIXEL_FORMAT_444YUV8 },
    { FC2_VIDEOMODE_320x240YUV422   , FC2_PIXEL_FORMAT_422YUV8 },
    { FC2_VIDEOMODE_640x480YUV411   , FC2_PIXEL_FORMAT_411YUV8 },
    { FC2_VIDEOMODE_640x480YUV422   , FC2_PIXEL_FORMAT_422YUV8 },
    { FC2_VIDEOMODE_640x480RGB      , FC2_PIXEL_FORMAT_RGB     },
    { FC2_VIDEOMODE_640x480Y8       , FC2_PIXEL_FORMAT_MONO8   },
    { FC2_VIDEOMODE_640x480Y16      , FC2_PIXEL_FORMAT_MONO16  },
    { FC2_VIDEOMODE_800x600YUV422   , FC2_PIXEL_FORMAT_422YUV8 },
    { FC2_VIDEOMODE_800x600RGB      , FC2_PIXEL_FORMAT_RGB8    },
    { FC2_VIDEOMODE_800x600Y8       , FC2_PIXEL_FORMAT_MONO8   },
    { FC2_VIDEOMODE_800x600Y16      , FC2_PIXEL_FORMAT_MONO16  },
    { FC2_VIDEOMODE_1024x768YUV422  , FC2_PIXEL_FORMAT_422YUV8 },
    { FC2_VIDEOMODE_1024x768RGB     , FC2_PIXEL_FORMAT_RGB8    },
    { FC2_VIDEOMODE_1024x768Y8      , FC2_PIXEL_FORMAT_MONO8   },
    { FC2_VIDEOMODE_1024x768Y16     , FC2_PIXEL_FORMAT_MONO16  },
    { FC2_VIDEOMODE_1280x960YUV422  , FC2_PIXEL_FORMAT_422YUV8 },
    { FC2_VIDEOMODE_1280x960RGB     , FC2_PIXEL_FORMAT_RGB8    },
    { FC2_VIDEOMODE_1280x960Y8      , FC2_PIXEL_FORMAT_MONO8   },
    { FC2_VIDEOMODE_1280x960Y16     , FC2_PIXEL_FORMAT_MONO16  },
    { FC2_VIDEOMODE_1600x1200YUV422 , FC2_PIXEL_FORMAT_422YUV8 },
    { FC2_VIDEOMODE_1600x1200RGB    , FC2_PIXEL_FORMAT_RGB8    },
    { FC2_VIDEOMODE_1600x1200Y8     , FC2_PIXEL_FORMAT_MONO8   },
    { FC2_VIDEOMODE_1600x1200Y16    , FC2_PIXEL_FORMAT_MONO16  },
};

/// Key: fc2PropertyInfo::pUnitAbbr
static const std::map<std::string, std::chrono::duration<double>> DURATION_VALUE =
{
    { "us", 1us },
    { "ms", 1ms },
    { "s",  1s }
};

DPTR_IMPL(FC2Imager)
{
    fc2Context context;
    fc2CameraInfo camInfo;

    std::map<fc2VideoMode, std::vector<fc2FrameRate>> videoModes;
    std::map<fc2Mode, fc2Format7Info> fmt7Modes;
    std::map<fc2PropertyType, fc2PropertyInfo> propertyInfo;

    FC2VideoMode currentVidMode;
    fc2FrameRate currentFrameRate;
    fc2PixelFormat currentPixFmt;

    /// Copy of the SHUTTER control; used for informing GUI about shutter range change when frame rate changes
    Control ctrlShutter;

    // Copies of controls; used for informing the GUI about changes in their availability
    Control ctrlWhiteBalanceRed, ctrlWhiteBalanceBlue;

    ROIValidator::ptr roiValidator; ///< Region of Interest validator

    /// Concerns the current video mode; used to disable ROI and return to full image size
    QSize maxFrameSize;

    QRect currentROI;

    bool temperatureAvailable;

    Control enumerateVideoModes();

    /// Returns a combo control listing all pixel formats for 'currentVidMode'
    Control enumerateCurrentModePixelFormats();

    /** Resets frame size, ROI validator and frame rate (if applicable). The actual video mode change
        comes into effect after the worker thread is restarted. */
    void changeVideoMode(const FC2VideoMode &newMode);

    Control getFrameRates(FC2VideoMode vidMode);

    void updateShutterCtrl();

    void createWhiteBalanceCtrls();

    Control getEmptyFrameRatesCtrl();

    /// Caller must check if the result is valid()
    Control createControlFromFC2Property(const fc2PropertyInfo &propInfo);


    LOG_C_SCOPE(FC2Imager);
};

/// Ensures both raw and absolute ranges satisfy: min < max
static void VerifyRanges(fc2PropertyInfo &propInfo)
{
    if (propInfo.min > propInfo.max)
        std::swap(propInfo.min, propInfo.max);

    if (propInfo.absMin > propInfo.absMax)
        std::swap(propInfo.absMin, propInfo.absMax);
}

static void UpdateRangeAndStep(Imager::Control &control, const fc2PropertyInfo &propInfo);

static void ForEachPossiblePixelFormat(const std::function<void (fc2PixelFormat)> &func)
{
    // TODO: implement handling of the commented out formats
    for (const auto pixFmt: {
            FC2_PIXEL_FORMAT_MONO8,
            FC2_PIXEL_FORMAT_RAW8,
            FC2_PIXEL_FORMAT_RGB8,

            //TODO: handle this
            // FC2_PIXEL_FORMAT_411YUV8,
            // FC2_PIXEL_FORMAT_422YUV8,
            // FC2_PIXEL_FORMAT_444YUV8,

            FC2_PIXEL_FORMAT_MONO16,
            FC2_PIXEL_FORMAT_RAW16,
            FC2_PIXEL_FORMAT_RGB16,

            //TODO: handle this
            // FC2_PIXEL_FORMAT_MONO12,
            // FC2_PIXEL_FORMAT_RAW12,
            /* FC2_PIXEL_FORMAT_422YUV8_JPEG*/ })
    {
        func(pixFmt);
    }
}

Imager::Control FC2Imager::Private::enumerateCurrentModePixelFormats()
{
    auto pixelFormats = Imager::Control{ ControlID::PixelFormat, "Pixel Format", Control::Combo };

    if (currentVidMode.isFormat7())
    {
        const auto &f7info = fmt7Modes[(fc2Mode)currentVidMode];

        ForEachPossiblePixelFormat(
            [&f7info, &pixelFormats](fc2PixelFormat pixFmt)
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
        const fc2VideoMode vidMode = vm.first;
        auto resolution = VID_MODE_RESOLUTION[vidMode];
        QString modeName = "%1x%2 %3"_q % (int)std::get<0>(resolution) % (int)std::get<1>(resolution) % PIXEL_FORMAT_NAME[VID_MODE_PIX_FMT[vidMode]];
        videoMode.add_choice_enum(modeName, (FC2VideoMode)vidMode);
    }

    for (const auto &f7m: fmt7Modes)
    {
        QString modeName = "%1x%2 (FMT7: %4)"_q % f7m.second.maxWidth
                                                % f7m.second.maxHeight
                                                % (f7m.second.mode - FC2_MODE_0);

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
        const auto frloc = videoModes.find((fc2VideoMode)vidMode);
        assert(frloc != videoModes.end());

        const auto &frList = frloc->second;

        for (const auto fr: frList)
            switch (fr)
            {
            case FC2_FRAMERATE_1_875: frameRatesCtrl.add_choice_enum("1.875 fps", fr); break;
            case FC2_FRAMERATE_3_75:  frameRatesCtrl.add_choice_enum("3.75 fps",  fr); break;
            case FC2_FRAMERATE_7_5:   frameRatesCtrl.add_choice_enum("7.5 fps",   fr); break;
            case FC2_FRAMERATE_15:    frameRatesCtrl.add_choice_enum("15 fps",    fr); break;
            case FC2_FRAMERATE_30:    frameRatesCtrl.add_choice_enum("30 fps",    fr); break;
            case FC2_FRAMERATE_60:    frameRatesCtrl.add_choice_enum("60 fps",    fr); break;
            case FC2_FRAMERATE_120:   frameRatesCtrl.add_choice_enum("120 fps",   fr); break;
            case FC2_FRAMERATE_240:   frameRatesCtrl.add_choice_enum("240 fps",   fr); break;
            }

        // Initially we set the highest frame rate, so report it as selected
        frameRatesCtrl.set_value_enum(*std::max_element(frList.begin(), frList.end()));

        return frameRatesCtrl;
    }
    else
        return getEmptyFrameRatesCtrl();
}

static fc2PixelFormat GetFirstSupportedPixelFormat(const fc2Format7Info &f7info)
{
    fc2PixelFormat first = FC2_UNSPECIFIED_PIXEL_FORMAT;
    bool firstAlreadySelected = false;

    ForEachPossiblePixelFormat([&](fc2PixelFormat pixFmt)
                               {
                                   if ((f7info.pixelFormatBitField & pixFmt) == pixFmt && !firstAlreadySelected)
                                   {
                                       first = pixFmt;
                                       firstAlreadySelected = true;
                                   }
                               });

    return first;
}

FC2Imager::FC2Imager(const fc2PGRGuid &guid, const ImageHandlerPtr &handler)
: Imager(handler), dptr()
{
    //FIXME: if a CHECK fails in Imager constructor, there is a segfault (instead of printing the caught exception)

    FC2_CHECK << fc2CreateContext(&d->context)
              << "fc2CreateContext";

    FC2_CHECK << fc2Connect(d->context, const_cast<fc2PGRGuid *>(&guid))
              << "fc2Connect";

    d->temperatureAvailable = false;

    FC2_CHECK << fc2GetCameraInfo(d->context, &d->camInfo)
              << "fc2GetCameraInfo";

    for (int vidMode = 0; vidMode < FC2_VIDEOMODE_FORMAT7; vidMode++)
        for (int frameRate = 0; frameRate < FC2_FRAMERATE_FORMAT7; frameRate++)
        {
            BOOL supported = FALSE;

            FC2_CHECK << fc2GetVideoModeAndFrameRateInfo(d->context, (fc2VideoMode)vidMode, (fc2FrameRate)frameRate, &supported)
                      << "fc2GetVideoModeAndFrameRateInfo";

            if (supported)
                d->videoModes[(fc2VideoMode)vidMode].push_back((fc2FrameRate)frameRate);
        }

    for (int fmt7Mode = FC2_MODE_0; fmt7Mode < FC2_NUM_MODES; fmt7Mode++)
    {
        fc2Format7Info f7info;
        f7info.mode = (fc2Mode)fmt7Mode;
        BOOL supported = FALSE;

        FC2_CHECK << fc2GetFormat7Info(d->context, &f7info, &supported)
                  << "fc2GetFormat7Info";

        if (supported)
            d->fmt7Modes[(fc2Mode)fmt7Mode] = f7info;
    }

    if (!d->videoModes.empty())
        d->changeVideoMode(d->videoModes.begin()->first);
    else
        d->changeVideoMode(d->fmt7Modes.begin()->first);

    if (d->currentVidMode.isFormat7())
    {
        const auto &f7info = d->fmt7Modes[(fc2Mode)d->currentVidMode];
        d->currentPixFmt = GetFirstSupportedPixelFormat(f7info);
    }
    else
        d->currentPixFmt = VID_MODE_PIX_FMT[(fc2VideoMode)d->currentVidMode];
}

FC2Imager::~FC2Imager()
{
    FC2_CHECK << fc2Disconnect(d->context)
              << "fc2Disconnect";

    FC2_CHECK << fc2DestroyContext(d->context)
              << "fc2DestroyContext";
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
    case FC2_INTERFACE_GIGE: interfaceStr = "Gigabit Ethernet"; break;
    case FC2_INTERFACE_IEEE1394: interfaceStr = "IEEE 1394"; break;
    case FC2_INTERFACE_USB_2: interfaceStr = "USB 2.0"; break;
    case FC2_INTERFACE_USB_3: interfaceStr = "USB 3.0"; break;
    default: interfaceStr = "unknown"; break;
    }
    properties << Imager::Properties::Property{ "Interface", interfaceStr };

    QString driverStr;
    switch (d->camInfo.driverType)
    {
    case FC2_DRIVER_1394_CAM:       driverStr = "PGRCam.sys"; break;
    case FC2_DRIVER_1394_PRO:       driverStr = "PGR1394.sys"; break;
    case FC2_DRIVER_1394_JUJU:      driverStr = "firewire_core"; break;
    case FC2_DRIVER_1394_VIDEO1394: driverStr = "video1394"; break;
    case FC2_DRIVER_1394_RAW1394:   driverStr = "raw1394"; break;
    case FC2_DRIVER_USB_NONE:       driverStr = "native"; break;
    case FC2_DRIVER_USB_CAM:        driverStr = "PGRUsbCam.sys"; break;
    case FC2_DRIVER_USB3_PRO:       driverStr = "PGRXHCI.sys"; break;
    case FC2_DRIVER_GIGE_NONE:      driverStr = "native"; break;
    case FC2_DRIVER_GIGE_FILTER:    driverStr = "PGRGigE.sys"; break;
    case FC2_DRIVER_GIGE_PRO:       driverStr = "PGRGigEPro.sys"; break;
    case FC2_DRIVER_GIGE_LWF:       driverStr = "PgrLwf.sys"; break;
    default: driverStr = "unknown"; break;
    }
    properties << Imager::Properties::Property{ "Driver", driverStr };

    properties << Imager::Properties::Property{ "Sensor", d->camInfo.sensorInfo };

    properties << Imager::Properties::Property{ "Resolution", d->camInfo.sensorResolution };

    properties << Imager::Properties::Property{ "Firmware Version", d->camInfo.firmwareVersion };

    properties << Imager::Properties::Property{ "Firmware Build Time", d->camInfo.firmwareBuildTime };

    if (d->camInfo.maximumBusSpeed != FC2_BUSSPEED_SPEED_UNKNOWN)
    {
        QString maxBusSpeedStr;
        switch (d->camInfo.maximumBusSpeed)
        {
        case FC2_BUSSPEED_S100:        maxBusSpeedStr = "100 Mb/s"; break;
        case FC2_BUSSPEED_S200:        maxBusSpeedStr = "200 Mb/s"; break;
        case FC2_BUSSPEED_S400:        maxBusSpeedStr = "400 Mb/s"; break;
        case FC2_BUSSPEED_S480:        maxBusSpeedStr = "480 Mb/s"; break;
        case FC2_BUSSPEED_S800:        maxBusSpeedStr = "800 Mb/s"; break;
        case FC2_BUSSPEED_S1600:       maxBusSpeedStr = "1.6 Gb/s"; break;
        case FC2_BUSSPEED_S3200:       maxBusSpeedStr = "3.2 Gb/s"; break;
        case FC2_BUSSPEED_S5000:       maxBusSpeedStr = "5.0 Gb/s"; break;
        case FC2_BUSSPEED_10BASE_T:    maxBusSpeedStr = "10Base-T"; break;
        case FC2_BUSSPEED_100BASE_T:   maxBusSpeedStr = "100Base-T"; break;
        case FC2_BUSSPEED_1000BASE_T:  maxBusSpeedStr = "1000Base-T"; break;
        case FC2_BUSSPEED_10000BASE_T: maxBusSpeedStr = "10000Base-T"; break;
        case FC2_BUSSPEED_S_FASTEST:   maxBusSpeedStr = "Fastest available"; break;
        case FC2_BUSSPEED_ANY:         maxBusSpeedStr = "Any available"; break;
        }

        properties << Imager::Properties::Property{ "Max. Bus Speed", maxBusSpeedStr };
    }

    if (d->camInfo.interfaceType == FC2_INTERFACE_GIGE)
    {
        QString macStr;
        macStr.sprintf("%02X:%02X:%02X:%02X:%02X:%02X", d->camInfo.macAddress.octets[0],
                                                        d->camInfo.macAddress.octets[1],
                                                        d->camInfo.macAddress.octets[2],
                                                        d->camInfo.macAddress.octets[3],
                                                        d->camInfo.macAddress.octets[4],
                                                        d->camInfo.macAddress.octets[5]);

        properties << Imager::Properties::Property{ "MAC",  macStr };

        auto ipFormatter = [](const fc2IPAddress &ip)
                           {
                               QString str;
                               str.sprintf("%d.%d.%d.%d", ip.octets[0], ip.octets[1], ip.octets[2], ip.octets[3]);
                               return str;
                           };

        properties << Imager::Properties::Property{ "IP Address", ipFormatter(d->camInfo.ipAddress) };

        properties << Imager::Properties::Property{ "Subnet Mask", ipFormatter(d->camInfo.subnetMask) };
    }

    fc2PropertyInfo propInfo;
    propInfo.type = FC2_TEMPERATURE;
    FC2_CHECK << fc2GetPropertyInfo(d->context, &propInfo)
              << "fc2GetPropertyInfo";
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
        const auto &fmt7 = fmt7Modes[(fc2Mode)newMode];

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
        const auto &res = VID_MODE_RESOLUTION[(fc2VideoMode)newMode];
        const int width  = std::get<0>(res);
        const int height = std::get<1>(res);

        currentROI = { 0, 0, width, height };

        roiValidator = std::make_shared<ROIValidator>(std::list<ROIValidator::Rule>{ });

        // Initially choose the highest frame rate
        currentFrameRate = *videoModes[(fc2VideoMode)newMode].rbegin();
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
        d->currentFrameRate = control.get_value_enum<fc2FrameRate>();
        startLive();
    }
    else if (control.id == ControlID::PixelFormat)
    {
        d->currentPixFmt = control.get_value_enum<fc2PixelFormat>();
        startLive();
    }
    else
    {
        fc2Property prop;

        if (control.id == ControlID::WhiteBalanceRed ||
            control.id == ControlID::WhiteBalanceBlue)
        {
            prop.type = FC2_WHITE_BALANCE;
        }
        else
            prop.type = (fc2PropertyType)control.id;

        FC2_CHECK << fc2GetProperty(d->context, &prop)
                  << "fc2GetProperty";

        // When the on/off or auto state of one of the white balance controls (Red, Blue) changes,
        // the other's does too (as it is in fact a single FC2 control)
        const bool wbAutoOnOffChanged =
            (control.id == ControlID::WhiteBalanceRed || control.id == ControlID::WhiteBalanceBlue) &&
            ((prop.autoManualMode ^ control.value_auto) || (prop.onOff ^ control.value_onOff));

        prop.autoManualMode = control.value_auto;
        prop.onOff = control.value_onOff;

        if (control.id == ControlID::WhiteBalanceRed)
        {
            prop.valueA = control.value.toInt();
        }
        else if (control.id == ControlID::WhiteBalanceBlue)
        {
            prop.valueB = control.value.toInt();
        }
        else
        {
            prop.absValue = control.value.toFloat();
            prop.valueA = control.value.toInt();
        }

        FC2_CHECK << fc2SetProperty(d->context, &prop)
                  << "fc2SetProperty";

        if (wbAutoOnOffChanged)
        {
            d->ctrlWhiteBalanceRed.value = prop.valueA;
            d->ctrlWhiteBalanceBlue.value = prop.valueB;

            for (Control *c: { &d->ctrlWhiteBalanceRed,
                               &d->ctrlWhiteBalanceBlue })
            {
                c->value_auto = prop.autoManualMode;
                c->value_onOff = prop.onOff;
            }

            emit changed(d->ctrlWhiteBalanceRed);
            emit changed(d->ctrlWhiteBalanceBlue);
        }
    }

    emit changed(control);

    if (d->ctrlShutter.valid() &&
        (FC2_FRAME_RATE == control.id ||
         ControlID::FrameRate    == control.id ||
         ControlID::VideoMode    == control.id ||
         ControlID::PixelFormat  == control.id))
    {
        // Changing frame rate, video mode or pixel format may change the shutter range; need to inform the GUI

        d->updateShutterCtrl();
        emit changed(d->ctrlShutter);
    }

    if (ControlID::VideoMode == control.id ||
        ControlID::PixelFormat == control.id)
    {
        // Some video modes/pixel formats may not support white balance;
        // createWhiteBalanceCtrls() will then return disabled controls
        d->createWhiteBalanceCtrls();
        emit changed(d->ctrlWhiteBalanceRed);
        emit changed(d->ctrlWhiteBalanceBlue);
    }
}

void FC2Imager::readTemperature()
{
    if (d->temperatureAvailable)
    {
        fc2Property prop;
        prop.type = FC2_TEMPERATURE;
        FC2_CHECK << fc2GetProperty(d->context, &prop)
                  << "fc2GetProperty";

        // Calculation as in CamSettingsPage.cpp from FlyCapture2 SDK. Strangely, both "absolute capable"
        // and "unit abbreviation" fields are not taken into account (e.g. on Chameleon3 CM3-U3-13S2M the indicated
        // unit is Celsius; yet, the formula below must be used anyway).

        double tempCelsius = prop.valueA / 10.0 - 273.15;

        emit temperature(tempCelsius);
    }
}

static void UpdateRangeAndStep(Imager::Control &control, const fc2PropertyInfo &propInfo)
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

Imager::Control FC2Imager::Private::createControlFromFC2Property(const fc2PropertyInfo &propInfo)
{
    Control control{ propInfo.type };
    control.type = Control::Type::Number;

    fc2Property prop;
    prop.type = propInfo.type;
    FC2_CHECK << fc2GetProperty(context, &prop)
              << "fc2GetProperty";

    switch (propInfo.type)
    {
        case FC2_BRIGHTNESS:      control.name = "Brightness"; break;

        // This is not "exposure time" (see FC2_SHUTTER for that);
        // instead, it regulates the desired overall image brightness by changing
        // values of shutter and gain - if they are set to auto
        case FC2_AUTO_EXPOSURE:   control.name = "Exposure"; break;

        case FC2_SHARPNESS:       control.name = "Sharpness"; break;

        case FC2_HUE:             control.name = "Hue"; break;
        case FC2_SATURATION:      control.name = "Saturation"; break;
        case FC2_GAMMA:           control.name = "Gamma"; break;

        case FC2_SHUTTER:
            control.name = "Shutter";
            break;

        case FC2_GAIN:            control.name = "Gain"; break;
        case FC2_IRIS:            control.name = "Iris"; break;
        case FC2_FOCUS:           control.name = "Focus"; break;
        case FC2_WHITE_BALANCE:   control.name = "White Balance"; break;
        case FC2_FRAME_RATE:      control.name = "Frame Rate"; break;
        case FC2_ZOOM:            control.name = "Zoom"; break;
        case FC2_PAN:             control.name = "Pan"; break;
        case FC2_TILT:            control.name = "Tilt"; break;

        // Control not yet supported
        default: control.name = ""; return control;
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
        FC2_CHECK << fc2SetProperty(context, &prop)
                  << "fc2SetProperty - enable absolute control";

        if (propInfo.readOutSupported)
        {
            FC2_CHECK << fc2GetProperty(context, &prop)
                      << "fc2GetProperty";

            control.value = prop.absValue;
        }
        else
            control.value = propInfo.absMin;

        if (propInfo.type == FC2_SHUTTER)
        {
            control.is_duration = true;
            const auto &unitLoc = DURATION_VALUE.find(propInfo.pUnitAbbr);
            if (unitLoc != DURATION_VALUE.end())
                control.duration_unit = unitLoc->second;
            else
                control.duration_unit = 1s;
        }
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

    return control;
}

Imager::Controls FC2Imager::controls() const
{
    Controls controls;

    controls.push_back(std::move(d->enumerateVideoModes()));

    controls.push_back(std::move(d->enumerateCurrentModePixelFormats()));

    controls.push_back(d->getFrameRates(d->currentVidMode));

    // FC2_WHITE_BALANCE is handled after this loop
    for (fc2PropertyType propType: { FC2_BRIGHTNESS,
                                     FC2_AUTO_EXPOSURE,
                                     FC2_SHARPNESS,
                                     FC2_HUE,
                                     FC2_SATURATION,
                                     FC2_GAMMA,
                                     FC2_IRIS,
                                     FC2_FOCUS,
                                     FC2_ZOOM,
                                     FC2_PAN,
                                     FC2_TILT,
                                     FC2_SHUTTER,
                                     FC2_GAIN,
                                     FC2_TRIGGER_MODE,
                                     FC2_TRIGGER_DELAY,
                                     FC2_FRAME_RATE })
    {
        fc2PropertyInfo propInfo;
        propInfo.type = propType;

        FC2_CHECK << fc2GetPropertyInfo(d->context, &propInfo)
                  << "fc2GetPropertyInfo";

        VerifyRanges(propInfo);

        if (propInfo.present)
        {
            d->propertyInfo[propType] = propInfo;

            auto control = d->createControlFromFC2Property(propInfo);
            if (!control.valid())
                continue;

            if (propInfo.type == FC2_SHUTTER)
                d->ctrlShutter = control; // See the comment for ctrlShutter

            controls.push_back(std::move(control));
        }
    }

    // Create white balance controls.
    // Even if white balance is not supported in current video mode, return the controls (in inactive state),
    // as we may need them after switching to another video mode (then they will be activated).
    d->createWhiteBalanceCtrls();
    controls.push_back(d->ctrlWhiteBalanceRed);
    controls.push_back(d->ctrlWhiteBalanceBlue);

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
    restart([this] { return std::make_shared<FC2ImagerWorker>(d->context, d->currentVidMode, d->currentFrameRate, d->currentPixFmt, d->currentROI); });
    qDebug() << "Video streaming started successfully";
}

void FC2Imager::Private::updateShutterCtrl()
{
    fc2PropertyInfo propInfo;
    propInfo.type = FC2_SHUTTER;

    FC2_CHECK << fc2GetPropertyInfo(context, &propInfo)
              << "fc2GetPropertyInfo - shutter";

    VerifyRanges(propInfo);

    UpdateRangeAndStep(ctrlShutter, propInfo);
}

void FC2Imager::Private::createWhiteBalanceCtrls()
{
    fc2PropertyInfo propInfo;
    propInfo.type = FC2_WHITE_BALANCE;
    FC2_CHECK << fc2GetPropertyInfo(context, &propInfo)
              << "fc2GetPropertyInfo - white balance";

    if (propInfo.present)
    {
        VerifyRanges(propInfo);

        auto control = createControlFromFC2Property(propInfo);

        fc2Property prop;
        prop.type = FC2_WHITE_BALANCE;
        FC2_CHECK << fc2GetProperty(context, &prop)
                  << "fc2GetProperty - white balance";

        ctrlWhiteBalanceRed = control;
        ctrlWhiteBalanceBlue = control;

        ctrlWhiteBalanceRed.value = prop.valueA;
        ctrlWhiteBalanceBlue.value = prop.valueB;
    }
    else
    {
        // return disabled white balance controls
        for (Control *ctrl: { &ctrlWhiteBalanceRed,
                              &ctrlWhiteBalanceBlue })
        {
            ctrl->value = 0;
            ctrl->supports_onOff = false;
            ctrl->supports_auto = false;
            ctrl->readonly = true;
        }
    }

    ctrlWhiteBalanceRed.name = "White Balance \u2013 Red"; // u2013 = en-dash
    ctrlWhiteBalanceRed.id = ControlID::WhiteBalanceRed;

    ctrlWhiteBalanceBlue.name = "White Balance \u2013 Blue";
    ctrlWhiteBalanceBlue.id = ControlID::WhiteBalanceBlue;
}
