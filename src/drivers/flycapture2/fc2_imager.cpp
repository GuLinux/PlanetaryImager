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

//#include <algorithm>
//#include <chrono>

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


DPTR_IMPL(FC2Imager)
{
    FlyCapture2::Camera cam;
    FlyCapture2::CameraInfo camInfo;

    /// Contains either a value from FlyCapture2::VideoMode, or a value from FlyCapture2::Mode (i.e. a Format7 mode) + FMT7_BASE
    class FC2VideoMode
    {
        static constexpr int FMT7_BASE = FlyCapture2::VideoMode::NUM_VIDEOMODES + 1;

        int mode;

    public:

        FC2VideoMode(): mode(-1) { }
        FC2VideoMode(const FC2VideoMode &) = default;
        FC2VideoMode(FlyCapture2::VideoMode vidMode): mode(vidMode) { }
        FC2VideoMode(FlyCapture2::Mode fmt7Mode): mode(fmt7Mode + FMT7_BASE) { }

        explicit operator FlyCapture2::VideoMode() const { return (FlyCapture2::VideoMode)mode; }
        explicit operator FlyCapture2::Mode()      const { return (FlyCapture2::Mode)(mode - FMT7_BASE); }
        explicit operator int()                    const { return mode; }

        bool isFormat7() const { return mode >= FMT7_BASE + FlyCapture2::MODE_0; }
    };

    std::map<FlyCapture2::VideoMode, std::vector<FlyCapture2::FrameRate>> videoModes;
    std::map<FlyCapture2::Mode, FlyCapture2::Format7Info> fmt7Modes;

    FC2VideoMode currentVidMode;
    FlyCapture2::PixelFormat currentPixFmt;
//
//    dc1394featureset_t features;
//    std::unordered_map<dc1394feature_t, bool> hasAbsoluteControl;
//
//    Properties properties;
//
    /// Copy of the SHUTTER control; used for informing GUI about shutter range change when frame rate changes
    Control ctrlShutter;

    ROIValidator::ptr roiValidator; ///< Region of Interest validator

    /// Concerns the current video mode; used to disable ROI and return to full image size
    QSize maxFrameSize;

    QRect currentROI;

    void updateWorkerExposureTimeout();

    Control enumerateVideoModes();

    /// Returns a combo control listing all pixel formats for 'currentVidMode'
    Control enumerateCurrentModePixelFormats();

    void getRawRange(FlyCapture2::PropertyType id, uint32_t &rawMin, uint32_t &rawMax);

    void getAbsoluteRange(FlyCapture2::PropertyType id, float &absMin, float &absMax);

    /** Resets frame size and ROI validator (if applicable). The actual video mode change
        comes into effect after the worker thread is restarted. */
    void changeVideoMode(const FC2VideoMode &newMode);

    /// Sets the highest supported frame rate
    void setHighestFrameRate(FlyCapture2::VideoMode vidMode);

    Control getFrameRates(FlyCapture2::VideoMode vidMode);

    void updateShutterCtrl();


    LOG_C_SCOPE(FC2Imager);
};

static void UpdateRangeAndStep(bool absoluteCapable, Imager::Control &control,
                               uint32_t rawMin, uint32_t rawMax,
                               float    absMin, float    absMax);

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

            // FlyCapture2::PIXEL_FORMAT_411YUV8,
            // FlyCapture2::PIXEL_FORMAT_422YUV8,
            // FlyCapture2::PIXEL_FORMAT_444YUV8,

            FlyCapture2::PIXEL_FORMAT_MONO16,
            FlyCapture2::PIXEL_FORMAT_RAW16,
            FlyCapture2::PIXEL_FORMAT_RGB16,

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
//
//Imager::Control IIDCImager::Private::getFrameRates(dc1394video_mode_t vidMode)
//{
//    const auto frloc = frameRates.find(vidMode);
//    assert(frloc != frameRates.end());
//
//    const dc1394framerates_t &fr = frloc->second;
//
//    auto frameRatesCtrl = Imager::Control{ ControlID::FrameRate, "Fixed Frame Rate", Control::Combo };
//
//    for (auto i = 0; i < fr.num; i++)
//        switch (fr.framerates[i])
//        {
//        case DC1394_FRAMERATE_1_875: frameRatesCtrl.add_choice_enum("1.875 fps", fr.framerates[i]); break;
//        case DC1394_FRAMERATE_3_75:  frameRatesCtrl.add_choice_enum("3.75 fps",  fr.framerates[i]); break;
//        case DC1394_FRAMERATE_7_5:   frameRatesCtrl.add_choice_enum("7.5 fps",   fr.framerates[i]); break;
//        case DC1394_FRAMERATE_15:    frameRatesCtrl.add_choice_enum("15 fps",    fr.framerates[i]); break;
//        case DC1394_FRAMERATE_30:    frameRatesCtrl.add_choice_enum("30 fps",    fr.framerates[i]); break;
//        case DC1394_FRAMERATE_60:    frameRatesCtrl.add_choice_enum("60 fps",    fr.framerates[i]); break;
//        case DC1394_FRAMERATE_120:   frameRatesCtrl.add_choice_enum("120 fps",   fr.framerates[i]); break;
//        case DC1394_FRAMERATE_240:   frameRatesCtrl.add_choice_enum("240 fps",   fr.framerates[i]); break;
//        }
//
//    // Initially we set the highest frame rate, so report it as selected
//    frameRatesCtrl.set_value_enum(*std::max_element(fr.framerates, fr.framerates + fr.num));
//
//    return frameRatesCtrl;
//}
//
////Q_DECLARE_METATYPE(IIDCImagerWorker::ImageType)
//
FC2Imager::FC2Imager(const FlyCapture2::PGRGuid &guid, const ImageHandler::ptr &handler)
: Imager(handler), dptr()
{
    //FIXME: if a CHECK fails in Imager constructor, there is a segfault (instead of printing the caught exception)

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
    {
        d->changeVideoMode(d->videoModes.begin()->first);
        d->setHighestFrameRate((FlyCapture2::VideoMode)d->currentVidMode);
    }
    else
        d->changeVideoMode(d->fmt7Modes.begin()->first);

    if (d->currentVidMode.isFormat7())
    {
        FlyCapture2::PixelFormat first = FlyCapture2::UNSPECIFIED_PIXEL_FORMAT;
        bool firstAlreadySelected = false;

        ForEachPossiblePixelFormat([&](FlyCapture2::PixelFormat pixFmt)
                                   {
                                       if (!firstAlreadySelected)
                                       {
                                           first = pixFmt;
                                           firstAlreadySelected = true;
                                       }
                                   });

        d->currentPixFmt = first;
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
        case FlyCapture2::BUSSPEED_S100:        maxBusSpeedStr = "100 Mbit/s"; break;
        case FlyCapture2::BUSSPEED_S200:        maxBusSpeedStr = "200 Mbit/s"; break;
        case FlyCapture2::BUSSPEED_S400:        maxBusSpeedStr = "400 Mbit/s"; break;
        case FlyCapture2::BUSSPEED_S480:        maxBusSpeedStr = "480 Mbit/s"; break;
        case FlyCapture2::BUSSPEED_S800:        maxBusSpeedStr = "800 Mbit/s"; break;
        case FlyCapture2::BUSSPEED_S1600:       maxBusSpeedStr = "1600 Mbit/s"; break;
        case FlyCapture2::BUSSPEED_S3200:       maxBusSpeedStr = "3200 Mbit/s"; break;
        case FlyCapture2::BUSSPEED_S5000:       maxBusSpeedStr = "5000 Mbit/s"; break;
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

    return properties;
}

QString FC2Imager::name() const
{
    return d->camInfo.modelName;
}

void FC2Imager::Private::changeVideoMode(const FC2VideoMode &newMode)
{
//    if (DC1394_TRUE == dc1394_is_video_mode_scalable(newMode))
//    {
//        const dc1394format7mode_t &fmt7 = fmt7Info[newMode];
//
//        roiValidator = std::make_shared<ROIValidator>(
//            std::list<ROIValidator::Rule>{ ROIValidator::x_multiple(fmt7.unit_pos_x),
//                                           ROIValidator::y_multiple(fmt7.unit_pos_y),
//                                           ROIValidator::width_multiple(fmt7.unit_size_x),
//                                           ROIValidator::height_multiple(fmt7.unit_size_y),
//                                           ROIValidator::within_rect(QRect{ 0, 0, (int)fmt7.max_size_x, (int)fmt7.max_size_y }) });
//
//        maxFrameSize = { (int)fmt7.max_size_x, (int)fmt7.max_size_y };
//
//        currentROI = { 0, 0, maxFrameSize.width(), maxFrameSize.height() };
//    }
//    else
//    {
//        uint32_t width, height;
//        IIDC_CHECK << dc1394_get_image_size_from_video_mode(camera.get(), newMode, &width, &height)
//                   << "Get image size from video mode";
//
//        currentROI = { 0, 0, (int)width, (int)height };
//
//        roiValidator = std::make_shared<ROIValidator>(std::list<ROIValidator::Rule>{ });
//    }
//
    currentVidMode = newMode;
}

void FC2Imager::setControl(const Imager::Control& control)
{
    LOG_F_SCOPE

//    if (control.id == ControlID::VideoMode)
//    {
//        const dc1394video_mode_t newMode = control.get_value_enum<dc1394video_mode_t>();
//
//        d->changeVideoMode(newMode);
//
//        startLive();
//
//        if (dc1394_is_video_mode_scalable(newMode))
//        {
//            // Scalable modes do not use fixed frame rates; inform the GUI to show an empty list
//            Imager::Control emptyFrameRatesCtrl{ ControlID::FrameRate, "Fixed Frame Rate", Control::Combo };
//            emptyFrameRatesCtrl.add_choice("N/A", 0);
//            emptyFrameRatesCtrl.set_value(0);
//            emit changed(emptyFrameRatesCtrl);
//        }
//        else
//        {
//            d->setHighestFrameRate(newMode);
//            emit changed(d->getFrameRates(newMode));
//        }
//    }
//    else if (control.id == ControlID::FrameRate)
//    {
//        if (!dc1394_is_video_mode_scalable(d->currentVidMode))
//        {
//            IIDC_CHECK << dc1394_video_set_framerate(d->camera.get(), control.get_value_enum<dc1394framerate_t>())
//                       << "Set frame rate";
//
//            // Changing frame rate changes the shutter range; need to inform the GUI
//            if (d->ctrlShutter.valid())
//            {
//                d->updateShutterCtrl();
//                emit changed(d->ctrlShutter);
//            }
//        }
//    }
//    else
//    {
//        if (control.supports_auto)
//        {
//            dc1394feature_mode_t mode;
//            IIDC_CHECK << dc1394_feature_get_mode(d->camera.get(), (dc1394feature_t)control.id, &mode)
//                       << "Get feature mode";
//
//            if ((DC1394_FEATURE_MODE_AUTO == mode) ^ control.value_auto)
//            {
//                IIDC_CHECK << dc1394_feature_set_mode(d->camera.get(), (dc1394feature_t)control.id, control.value_auto ? DC1394_FEATURE_MODE_AUTO
//                                                                                                                       : DC1394_FEATURE_MODE_MANUAL)
//                           << "Set feature mode";
//            }
//        }
//
//        if (control.supports_onOff)
//        {
//            dc1394switch_t onOffState;
//            IIDC_CHECK << dc1394_feature_get_power(d->camera.get(), (dc1394feature_t)control.id, &onOffState)
//                       << "Get feature on/off state";
//
//            if ((DC1394_ON == onOffState) ^ control.value_onOff)
//            {
//                IIDC_CHECK << dc1394_feature_set_power(d->camera.get(), (dc1394feature_t)control.id, control.value_onOff ? DC1394_ON
//                                                                                                                         : DC1394_OFF)
//                           << "Set feature on/off state";
//            }
//        }
//
//        if ((!control.supports_onOff || control.value_onOff) &&
//            !(control.supports_auto  && control.value_auto))
//        {
//            if (d->hasAbsoluteControl[(dc1394feature_t)control.id])
//            {
//                IIDC_CHECK << dc1394_feature_set_absolute_value(d->camera.get(), (dc1394feature_t)control.id, control.value.toFloat())
//                           << "Set feature absolute value";
//            }
//            else
//            {
//                IIDC_CHECK << dc1394_feature_set_value(d->camera.get(), (dc1394feature_t)control.id, control.value.toInt())
//                           << "Set feature value";
//            }
//        }
//    }
//
//    emit changed(control);
//
//    if (DC1394_FEATURE_FRAME_RATE == control.id && d->ctrlShutter.valid())
//    {
//        // Changing frame rate changes the shutter range; need to inform the GUI
//
//        d->updateShutterCtrl();
//        emit changed(d->ctrlShutter);
//    }
}

void FC2Imager::readTemperature()
{
}
//
//
//void IIDCImager::Private::getRawRange(dc1394feature_t id, uint32_t &rawMin, uint32_t &rawMax)
//{
//    IIDC_CHECK << dc1394_feature_get_boundaries(camera.get(), id, &rawMin, &rawMax)
//               << "Get feature boundaries";
//
//    if (rawMin > rawMax)
//        std::swap(rawMin, rawMax);
//}
//
//void IIDCImager::Private::getAbsoluteRange(dc1394feature_t id, float &absMin, float &absMax)
//{
//    IIDC_CHECK << dc1394_feature_get_absolute_boundaries(camera.get(), id, &absMin, &absMax)
//               << "Get feature absolute boundaries";
//
//    if (absMin > absMax)
//        std::swap(absMin, absMax);
//}
//
//static void UpdateRangeAndStep(bool absoluteCapable, Imager::Control &control,
//                               uint32_t rawMin, uint32_t rawMax,
//                               float    absMin, float    absMax)
//{
//    if (absoluteCapable)
//    {
//        if (rawMax != rawMin)
//            control.range.step = (absMax - absMin) / (rawMax - rawMin + 1);
//        else
//            control.range.step = 0;
//
//        control.range.min = absMin;
//        control.range.max = absMax;
//    }
//    else
//    {
//        control.range.min = rawMin;
//        control.range.max = rawMax;
//        control.range.step = (rawMin != rawMax ? 1 : 0);
//    }
//}
//
Imager::Controls FC2Imager::controls() const
{
    Controls controls;

    controls.push_back(std::move(d->enumerateVideoModes()));

    controls.push_back(std::move(d->enumerateCurrentModePixelFormats()));

//
//    if (DC1394_FALSE == dc1394_is_video_mode_scalable(d->currentVidMode))
//        controls.push_back(d->getFrameRates(d->currentVidMode));
//
//    for (const dc1394feature_info_t &feature: d->features.feature)
//        if (DC1394_TRUE == feature.available)
//        {
//            Control control{ feature.id };
//            control.type = Control::Type::Number;
//
//            switch (feature.id)
//            {
//                case DC1394_FEATURE_BRIGHTNESS:      control.name = "Brightness"; break;
//
//                // This is not "exposure time" (see DC1394_FEATURE_SHUTTER for that);
//                // instead, it regulates the desired overall image brightness by changing
//                // values of shutter and gain - if they are set to auto
//                case DC1394_FEATURE_EXPOSURE:        control.name = "Exposure"; break;
//
//                case DC1394_FEATURE_SHARPNESS:       control.name = "Sharpness"; break;
//
//                //TODO: this is in fact a pair of controls
//                //case DC1394_FEATURE_WHITE_BALANCE:   control.name = "White balance"; break;
//
//                case DC1394_FEATURE_HUE:             control.name = "Hue"; break;
//                case DC1394_FEATURE_SATURATION:      control.name = "Saturation"; break;
//                case DC1394_FEATURE_GAMMA:           control.name = "Gamma"; break;
//
//                case DC1394_FEATURE_SHUTTER:
//                    control.name = "Shutter";
//                    control.is_exposure = true;
//                    control.is_duration = true;
//                    // No way to check the unit using IIDC API, but PGR Firefly MV (FMVU-03MTM) and Chameleon3 (CM3-U3-13S2M) both use seconds
//                    control.duration_unit = 1s;
//                    break;
//
//                case DC1394_FEATURE_GAIN:            control.name = "Gain"; break;
//                case DC1394_FEATURE_IRIS:            control.name = "Iris"; break;
//                case DC1394_FEATURE_FOCUS:           control.name = "Focus"; break;
//                case DC1394_FEATURE_TEMPERATURE:     control.name = "Temperature"; break;
//
//                // TODO: requires special handling
//                // case DC1394_FEATURE_TRIGGER:         control.name = "Trigger"; break;
//
//                // TODO: uncomment when DC1394_FEATURE_TRIGGER is implemented
//                // case DC1394_FEATURE_TRIGGER_DELAY:
//                //     control.name = "Trigger delay";
//                //     control.is_duration = true;
//                //     break;
//
//                //TODO: this is in fact a triple of controls
//                //case DC1394_FEATURE_WHITE_SHADING:   control.name = "White shading"; break;
//
//                case DC1394_FEATURE_FRAME_RATE:      control.name = "Frame Rate"; break;
//                case DC1394_FEATURE_ZOOM:            control.name = "Zoom"; break;
//                case DC1394_FEATURE_PAN:             control.name = "Pan"; break;
//                case DC1394_FEATURE_TILT:            control.name = "Tilt"; break;
//                case DC1394_FEATURE_OPTICAL_FILTER:  control.name = "Optical Filter"; break;
//                case DC1394_FEATURE_CAPTURE_SIZE:    control.name = "Capture Size"; break;
//                case DC1394_FEATURE_CAPTURE_QUALITY: control.name = "Capture Quality"; break;
//
//                default: continue;
//            }
//
//            control.supports_auto = (std::find(feature.modes.modes, feature.modes.modes + feature.modes.num, DC1394_FEATURE_MODE_AUTO)
//                                       != feature.modes.modes + feature.modes.num);
//
//            control.readonly = (0 == feature.modes.num &&
//                                DC1394_TRUE == feature.readout_capable);
//
//            control.supports_onOff = (DC1394_TRUE == feature.on_off_capable);
//            if (control.supports_onOff)
//            {
//                dc1394switch_t currOnOff;
//                IIDC_CHECK << dc1394_feature_get_power(d->camera.get(), feature.id, &currOnOff)
//                           << "Get feature on/off";
//                control.value_onOff = (DC1394_ON == currOnOff);
//            }
//
//            dc1394feature_mode_t currMode;
//            IIDC_CHECK << dc1394_feature_get_mode(d->camera.get(), feature.id, &currMode)
//                        << "Get feature mode";
//            control.value_auto = (DC1394_FEATURE_MODE_AUTO == currMode);
//
//            float absMin, absMax;
//            uint32_t rawMin, rawMax;
//
//            d->getRawRange(feature.id, rawMin, rawMax);
//
//            // A feature is "absolute control-capable", if its value can be set using
//            // floating-point arguments, not just the integer "raw/driver" values.
//            // E.g. SHUTTER can be set in fractional "absolute" values expressed in seconds.
//            dc1394bool_t absoluteCapable;
//            IIDC_CHECK << dc1394_feature_has_absolute_control(d->camera.get(), feature.id, &absoluteCapable)
//                       << "Get feature absolute capability";
//
//            if (DC1394_TRUE == absoluteCapable)
//            {
//                d->hasAbsoluteControl[feature.id] = true;
//
//                control.decimals = 6;
//
//                IIDC_CHECK << dc1394_feature_set_absolute_control(d->camera.get(), feature.id, DC1394_ON)
//                           << "Set feature absolute control";
//
//                d->getAbsoluteRange(feature.id, absMin, absMax);
//
//                if (DC1394_TRUE == feature.readout_capable)
//                {
//                    float fval;
//                    IIDC_CHECK << dc1394_feature_get_absolute_value(d->camera.get(), feature.id, &fval)
//                               << "Get feature absolute value";
//
//                    control.value = fval;
//                }
//                else
//                    control.value = fmin;
//            }
//            else
//            {
//                d->hasAbsoluteControl[feature.id] = false;
//
//                control.decimals = 0;
//
//                if (DC1394_TRUE == feature.readout_capable)
//                {
//                    uint32_t rawVal;
//                    IIDC_CHECK << dc1394_feature_get_value(d->camera.get(), feature.id, &rawVal)
//                               << "Get feature value";
//
//                    control.value = rawVal;
//                }
//                else
//                    control.value = rawMin;
//            }
//
//            UpdateRangeAndStep(d->hasAbsoluteControl[feature.id], control, rawMin, rawMax, absMin, absMax);
//
//            if (DC1394_FEATURE_SHUTTER == feature.id)
//                d->ctrlShutter = control; // See the comment for ctrlShutter
//
//            controls.push_back(std::move(control));
//        }
//

    return controls;
}


void FC2Imager::clearROI()
{
//    setROI(QRect{ 0, 0, d->maxFrameSize.width(), d->maxFrameSize.height() });
}

void FC2Imager::setROI(const QRect &roi)
{
//    d->currentROI = d->roiValidator->validate(roi, QRect{ });
//    startLive();
}


void FC2Imager::startLive()
{
    restart([this] { return std::make_shared<FC2ImagerWorker>(d->cam, d->currentROI); });
    qDebug() << "Video streaming started successfully";
}

void FC2Imager::Private::setHighestFrameRate(FlyCapture2::VideoMode vidMode)
{
//    const dc1394framerates_t &f = frameRates[vidMode];
//
//    const dc1394framerate_t *highest = std::max_element(f.framerates, f.framerates + f.num);
//
//    IIDC_CHECK << dc1394_video_set_framerate(camera.get(), *highest)
//               << "Set frame rate";
}
//
//void IIDCImager::Private::updateShutterCtrl()
//{
//    uint32_t rawMin, rawMax;
//    float absMin, absMax;
//
//    getRawRange(DC1394_FEATURE_SHUTTER, rawMin, rawMax);
//
//    const bool hasAbsControl = hasAbsoluteControl[DC1394_FEATURE_SHUTTER];
//    if (hasAbsControl)
//        getAbsoluteRange(DC1394_FEATURE_SHUTTER, absMin, absMax);
//
//    UpdateRangeAndStep(hasAbsControl, ctrlShutter, rawMin, rawMax, absMin, absMax);
//}
