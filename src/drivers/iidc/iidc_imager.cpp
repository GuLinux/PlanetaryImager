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
#include <chrono>
#include "commons/utils.h"
#include "drivers/roi.h"
#include "iidc_deleters.h"
#include "iidc_exception.h"
#include "iidc_imager.h"
#include "iidc_worker.h"
#include <map>
#include <QRect>
#include "Qt/strings.h"
#include <unordered_map>


using namespace std::chrono;

enum ControlID: qlonglong
{
    VideoMode = DC1394_FEATURE_MAX + 1,

    // Used to select one of the fixed framerates from the 'dc1394framerate_t' enum (only for non-scalable = non-Format7 modes);
    // a camera may also support DC1394_FEATURE_FRAME_RATE, which can change the frame rate independently and with finer granularity
    FrameRate
};

static std::map<dc1394color_coding_t, const char *> COLOR_CODING_NAME
{
    { DC1394_COLOR_CODING_MONO8,   "Mono 8-bit"           },
    { DC1394_COLOR_CODING_YUV411,  "YUV411"               },
    { DC1394_COLOR_CODING_YUV422,  "YUV422"               },
    { DC1394_COLOR_CODING_YUV444,  "YUV444"               },
    { DC1394_COLOR_CODING_RGB8,    "RGB 8-bit"            },
    { DC1394_COLOR_CODING_MONO16,  "Mono 16-bit"          },
    { DC1394_COLOR_CODING_RGB16,   "RGB 16-bit"           },
    { DC1394_COLOR_CODING_MONO16S, "Mono 16-bit (signed)" },
    { DC1394_COLOR_CODING_RGB16S,  "RGB 16-bit (signed)"  },
    { DC1394_COLOR_CODING_RAW8,    "RAW 8-bit"            },
    { DC1394_COLOR_CODING_RAW16,   "RAW 16-bit"           },
};

DPTR_IMPL(IIDCImager)
{
    QString cameraName;
    QString cameraVendor;

    std::unique_ptr<dc1394camera_t, Deleters::camera> camera;

    dc1394video_modes_t videoModes;
    dc1394video_mode_t currentVidMode;
    std::unordered_map<dc1394video_mode_t, dc1394framerates_t> frameRates; ///< Key: non-scalable video mode from 'videoModes'
    std::unordered_map<dc1394video_mode_t, dc1394format7mode_t> fmt7Info; ///< Key: scalable video mode form 'videoModes'

    dc1394featureset_t features;
    std::unordered_map<dc1394feature_t, bool> hasAbsoluteControl;

    Properties properties;

    /// Copy of the SHUTTER control; used for informing GUI about shutter range change when frame rate changes
    Control ctrlShutter;

    ROIValidator::ptr roiValidator; ///< Region of Interest validator

    /// Concerns the current video mode; used to disable ROI and return to full image size
    QSize maxFrameSize;

    QRect currentROI;

    void updateWorkerExposureTimeout();

    Control enumerateVideoModes();

    void getRawRange(dc1394feature_t id, uint32_t &rawMin, uint32_t &rawMax);

    void getAbsoluteRange(dc1394feature_t id, float &absMin, float &absMax);

    void changeVideoMode(dc1394video_mode_t newMode);

    /// If 'vidMode' supports a fixed set of frame rates, sets the highest supported frame rate
    void setHighestFrameRate(dc1394video_mode_t vidMode);

    Control getFrameRates(dc1394video_mode_t vidMode);

    void updateShutterCtrl();


    LOG_C_SCOPE(IIDCImager);
};

static void UpdateRangeAndStep(bool absoluteCapable, Imager::Control &control,
                               uint32_t rawMin, uint32_t rawMax,
                               float    absMin, float    absMax);

void IIDCImager::Private::updateWorkerExposureTimeout()
{
//TODO: implement this
}

Imager::Control IIDCImager::Private::enumerateVideoModes()
{
    auto videoMode = Imager::Control{ ControlID::VideoMode, "Video Mode", Control::Combo };

    for (uint32_t i = 0; i < videoModes.num; i++)
    {
        const dc1394video_mode_t vidMode = videoModes.modes[i];

        uint32_t width, height;
        IIDC_CHECK << dc1394_get_image_size_from_video_mode(camera.get(), vidMode, &width, &height)
                   << "Get image size from video mode";

        dc1394color_coding_t coding;
        IIDC_CHECK << dc1394_get_color_coding_from_video_mode(camera.get(), vidMode, &coding)
                   << "Get color coding from video mode";

        // FIXME: does not compile if COLOR_CODING_NAME is 'const'; possibly a problem with _q or %
        QString modeName = "%1x%2 %3"_q % width % height % COLOR_CODING_NAME[coding];

        if (vidMode >= DC1394_VIDEO_MODE_FORMAT7_MIN &&
            vidMode <= DC1394_VIDEO_MODE_FORMAT7_MAX)
        {
            // Format7 modes may have additional capabilities:
            //   - Frame size other than the fixed sizes in 'dc1394video_mode_t' enum
            //   - max frame rate different than the predefined ones in 'dc1394framerate_t' enum
            //   - ROI support
            //   - various modes of pixel binning
            //
            // Some cameras may report only Format7 modes.

            const dc1394format7mode_t &fmt7 = fmt7Info[vidMode];

            modeName = "%1x%2 %3 (FMT7: %4)"_q % fmt7.max_size_x % fmt7.max_size_y
                                               % COLOR_CODING_NAME[coding] %  (vidMode - DC1394_VIDEO_MODE_FORMAT7_0);
        }

        videoMode.add_choice_enum(modeName, vidMode);
    }
    videoMode.set_value_enum(currentVidMode);

    return videoMode;
}

Imager::Control IIDCImager::Private::getFrameRates(dc1394video_mode_t vidMode)
{
    const auto frloc = frameRates.find(vidMode);
    assert(frloc != frameRates.end());

    const dc1394framerates_t &fr = frloc->second;

    auto frameRatesCtrl = Imager::Control{ ControlID::FrameRate, "Fixed Frame Rate", Control::Combo };

    for (auto i = 0; i < fr.num; i++)
        switch (fr.framerates[i])
        {
        case DC1394_FRAMERATE_1_875: frameRatesCtrl.add_choice_enum("1.875 fps", fr.framerates[i]); break;
        case DC1394_FRAMERATE_3_75:  frameRatesCtrl.add_choice_enum("3.75 fps",  fr.framerates[i]); break;
        case DC1394_FRAMERATE_7_5:   frameRatesCtrl.add_choice_enum("7.5 fps",   fr.framerates[i]); break;
        case DC1394_FRAMERATE_15:    frameRatesCtrl.add_choice_enum("15 fps",    fr.framerates[i]); break;
        case DC1394_FRAMERATE_30:    frameRatesCtrl.add_choice_enum("30 fps",    fr.framerates[i]); break;
        case DC1394_FRAMERATE_60:    frameRatesCtrl.add_choice_enum("60 fps",    fr.framerates[i]); break;
        case DC1394_FRAMERATE_120:   frameRatesCtrl.add_choice_enum("120 fps",   fr.framerates[i]); break;
        case DC1394_FRAMERATE_240:   frameRatesCtrl.add_choice_enum("240 fps",   fr.framerates[i]); break;
        }

    // Initially we set the highest frame rate, so report it as selected
    frameRatesCtrl.set_value_enum(*std::max_element(fr.framerates, fr.framerates + fr.num));

    return frameRatesCtrl;
}

//Q_DECLARE_METATYPE(IIDCImagerWorker::ImageType)

IIDCImager::IIDCImager(std::unique_ptr<dc1394camera_t, Deleters::camera> camera, const ImageHandler::ptr &handler,
                       const QString &cameraName, const QString &cameraVendor)
: Imager(handler), dptr(cameraName, cameraVendor)
{
    d->camera = std::move(camera);

    IIDC_CHECK << dc1394_video_get_supported_modes(d->camera.get(), &d->videoModes)
               << "Get supported video modes";

    for (auto i = 0; i < d->videoModes.num; i++)
    {
        const dc1394video_mode_t vidMode = d->videoModes.modes[i];

        if (!dc1394_is_video_mode_scalable(vidMode))
        {
            auto modeFrameRates = d->frameRates.emplace(vidMode, dc1394framerates_t{ });
            IIDC_CHECK << dc1394_video_get_supported_framerates(d->camera.get(), vidMode, &modeFrameRates.first->second)
                       << "Get supported frame rates";
        }
        else
        {
            auto fmt7loc = d->fmt7Info.emplace(vidMode, dc1394format7mode_t{ });
            dc1394format7mode_t &fmt7 = fmt7loc.first->second;
            fmt7.present = DC1394_TRUE;

            IIDC_CHECK << dc1394_format7_get_mode_info(d->camera.get(), vidMode, &fmt7)
                       << "Get Format7 mode info";
        }
    }

    d->currentVidMode = d->videoModes.modes[0];
    d->changeVideoMode(d->currentVidMode);
    d->setHighestFrameRate(d->currentVidMode);

    IIDC_CHECK << dc1394_feature_get_all(d->camera.get(), &d->features)
               << "Get all features";

    connect(this, &Imager::exposure_changed, this, std::bind(&Private::updateWorkerExposureTimeout, d.get()));
}

IIDCImager::~IIDCImager()
{
}


Imager::Properties IIDCImager::properties() const
{
    auto properties = Imager::Properties();
    properties << LiveStream;

    properties << Imager::Properties::Property{ "Vendor", d->cameraVendor };

    auto vmEnd = d->videoModes.modes + d->videoModes.num;
    if (vmEnd != std::find_if(&d->videoModes.modes[0], vmEnd, [](const dc1394video_mode_t &vidMode) { return DC1394_TRUE == dc1394_is_video_mode_scalable(vidMode); }))
        properties << ROI;

    QString iidcVersion;
    switch (d->camera.get()->iidc_version)
    {
    case DC1394_IIDC_VERSION_1_04:   iidcVersion = "1.04"; break;
    case DC1394_IIDC_VERSION_1_20:   iidcVersion = "1.20"; break;
    case DC1394_IIDC_VERSION_PTGREY: iidcVersion = "Point Grey"; break;
    case DC1394_IIDC_VERSION_1_30:   iidcVersion = "1.30"; break;
    case DC1394_IIDC_VERSION_1_31:   iidcVersion = "1.31"; break;
    case DC1394_IIDC_VERSION_1_32:   iidcVersion = "1.32"; break;
    case DC1394_IIDC_VERSION_1_33:   iidcVersion = "1.33"; break;
    case DC1394_IIDC_VERSION_1_34:   iidcVersion = "1.34"; break;
    case DC1394_IIDC_VERSION_1_35:   iidcVersion = "1.35"; break;
    case DC1394_IIDC_VERSION_1_36:   iidcVersion = "1.36"; break;
    case DC1394_IIDC_VERSION_1_37:   iidcVersion = "1.37"; break;
    case DC1394_IIDC_VERSION_1_38:   iidcVersion = "1.38"; break;
    case DC1394_IIDC_VERSION_1_39:   iidcVersion = "1.39"; break;
    }
    properties << Imager::Properties::Property{ "IIDC version", iidcVersion };

    properties << Imager::Properties::Property{ "Firmware version: ", QString::number(d->camera.get()->unit_sw_version, 10) + "." +
                                                                      QString::number(d->camera.get()->unit_sub_sw_version, 10) };

    const auto maxImageWidth = std::max_element(d->fmt7Info.begin(), d->fmt7Info.end(),
                                          [](const auto &i1, const auto &i2)
                                          { return i1.second.max_size_x < i2.second.max_size_x; });

    const auto maxImageHeight = std::max_element(d->fmt7Info.begin(), d->fmt7Info.end(),
                                          [](const auto &i1, const auto &i2)
                                          { return i1.second.max_size_y < i2.second.max_size_y; });

    properties.set_resolution({ (int)maxImageWidth->second.max_size_x,
                                (int)maxImageHeight->second.max_size_y });


    return properties;
}

QString IIDCImager::name() const
{
    return d->cameraName;
}

void IIDCImager::Private::changeVideoMode(dc1394video_mode_t newMode)
{
    if (DC1394_TRUE == dc1394_is_video_mode_scalable(newMode))
    {
        const dc1394format7mode_t &fmt7 = fmt7Info[newMode];

        roiValidator = std::make_shared<ROIValidator>(
            std::list<ROIValidator::Rule>{ ROIValidator::x_multiple(fmt7.unit_pos_x),
                                           ROIValidator::y_multiple(fmt7.unit_pos_y),
                                           ROIValidator::width_multiple(fmt7.unit_size_x),
                                           ROIValidator::height_multiple(fmt7.unit_size_y),
                                           ROIValidator::within_rect(QRect{ 0, 0, (int)fmt7.max_size_x, (int)fmt7.max_size_y }) });

        maxFrameSize = { (int)fmt7.max_size_x, (int)fmt7.max_size_y };

        currentROI = { 0, 0, maxFrameSize.width(), maxFrameSize.height() };
    }
    else
    {
        uint32_t width, height;
        IIDC_CHECK << dc1394_get_image_size_from_video_mode(camera.get(), newMode, &width, &height)
                   << "Get image size from video mode";

        currentROI = { 0, 0, (int)width, (int)height };

        roiValidator = std::make_shared<ROIValidator>(std::list<ROIValidator::Rule>{ });
    }

    currentVidMode = newMode;
}

void IIDCImager::setControl(const Imager::Control& control)
{
    LOG_F_SCOPE

    if (control.id == ControlID::VideoMode)
    {
        const dc1394video_mode_t newMode = control.get_value_enum<dc1394video_mode_t>();

        d->changeVideoMode(newMode);

        startLive();

        if (dc1394_is_video_mode_scalable(newMode))
        {
            // Scalable modes do not use fixed frame rates; inform the GUI to show an empty list
            Imager::Control emptyFrameRatesCtrl{ ControlID::FrameRate, "Fixed Frame Rate", Control::Combo };
            emptyFrameRatesCtrl.add_choice("N/A", 0);
            emptyFrameRatesCtrl.set_value(0);
            emit changed(emptyFrameRatesCtrl);
        }
        else
        {
            d->setHighestFrameRate(newMode);
            emit changed(d->getFrameRates(newMode));
        }
    }
    else if (control.id == ControlID::FrameRate)
    {
        if (!dc1394_is_video_mode_scalable(d->currentVidMode))
        {
            IIDC_CHECK << dc1394_video_set_framerate(d->camera.get(), control.get_value_enum<dc1394framerate_t>())
                       << "Set frame rate";

            // Changing frame rate changes the shutter range; need to inform the GUI
            if (d->ctrlShutter.valid())
            {
                d->updateShutterCtrl();
                emit changed(d->ctrlShutter);
            }
        }
    }
    else
    {
        if (control.supports_auto)
        {
            dc1394feature_mode_t mode;
            IIDC_CHECK << dc1394_feature_get_mode(d->camera.get(), (dc1394feature_t)control.id, &mode)
                       << "Get feature mode";

            if ((DC1394_FEATURE_MODE_AUTO == mode) ^ control.value_auto)
            {
                IIDC_CHECK << dc1394_feature_set_mode(d->camera.get(), (dc1394feature_t)control.id, control.value_auto ? DC1394_FEATURE_MODE_AUTO
                                                                                                                       : DC1394_FEATURE_MODE_MANUAL)
                           << "Set feature mode";
            }
        }

        if (control.supports_onOff)
        {
            dc1394switch_t onOffState;
            IIDC_CHECK << dc1394_feature_get_power(d->camera.get(), (dc1394feature_t)control.id, &onOffState)
                       << "Get feature on/off state";

            if ((DC1394_ON == onOffState) ^ control.value_onOff)
            {
                IIDC_CHECK << dc1394_feature_set_power(d->camera.get(), (dc1394feature_t)control.id, control.value_onOff ? DC1394_ON
                                                                                                                         : DC1394_OFF)
                           << "Set feature on/off state";
            }
        }

        if ((!control.supports_onOff || control.value_onOff) &&
            !(control.supports_auto  && control.value_auto))
        {
            if (d->hasAbsoluteControl[(dc1394feature_t)control.id])
            {
                IIDC_CHECK << dc1394_feature_set_absolute_value(d->camera.get(), (dc1394feature_t)control.id, control.value.toFloat())
                           << "Set feature absolute value";
            }
            else
            {
                IIDC_CHECK << dc1394_feature_set_value(d->camera.get(), (dc1394feature_t)control.id, control.value.toInt())
                           << "Set feature value";
            }
        }
    }

    emit changed(control);

    if (DC1394_FEATURE_FRAME_RATE == control.id && d->ctrlShutter.valid())
    {
        // Changing frame rate changes the shutter range; need to inform the GUI

        d->updateShutterCtrl();
        emit changed(d->ctrlShutter);
    }
}

void IIDCImager::readTemperature()
{
}


void IIDCImager::Private::getRawRange(dc1394feature_t id, uint32_t &rawMin, uint32_t &rawMax)
{
    IIDC_CHECK << dc1394_feature_get_boundaries(camera.get(), id, &rawMin, &rawMax)
               << "Get feature boundaries";

    if (rawMin > rawMax)
        std::swap(rawMin, rawMax);
}

void IIDCImager::Private::getAbsoluteRange(dc1394feature_t id, float &absMin, float &absMax)
{
    IIDC_CHECK << dc1394_feature_get_absolute_boundaries(camera.get(), id, &absMin, &absMax)
               << "Get feature absolute boundaries";

    if (absMin > absMax)
        std::swap(absMin, absMax);
}

static void UpdateRangeAndStep(bool absoluteCapable, Imager::Control &control,
                               uint32_t rawMin, uint32_t rawMax,
                               float    absMin, float    absMax)
{
    if (absoluteCapable)
    {
        if (rawMax != rawMin)
            control.range.step = (absMax - absMin) / (rawMax - rawMin + 1);
        else
            control.range.step = 0;

        control.range.min = absMin;
        control.range.max = absMax;
    }
    else
    {
        control.range.min = rawMin;
        control.range.max = rawMax;
        control.range.step = (rawMin != rawMax ? 1 : 0);
    }
}

Imager::Controls IIDCImager::controls() const
{
    Controls controls;

    controls.push_back(std::move(d->enumerateVideoModes()));

    if (DC1394_FALSE == dc1394_is_video_mode_scalable(d->currentVidMode))
        controls.push_back(d->getFrameRates(d->currentVidMode));

    for (const dc1394feature_info_t &feature: d->features.feature)
        if (DC1394_TRUE == feature.available)
        {
            Control control{ feature.id };
            control.type = Control::Type::Number;

            switch (feature.id)
            {
                case DC1394_FEATURE_BRIGHTNESS:      control.name = "Brightness"; break;

                // This is not "exposure time" (see DC1394_FEATURE_SHUTTER for that);
                // instead, it regulates the desired overall image brightness by changing
                // values of shutter and gain - if they are set to auto
                case DC1394_FEATURE_EXPOSURE:        control.name = "Exposure"; break;

                case DC1394_FEATURE_SHARPNESS:       control.name = "Sharpness"; break;

                //TODO: this is in fact a pair of controls
                //case DC1394_FEATURE_WHITE_BALANCE:   control.name = "White balance"; break;

                case DC1394_FEATURE_HUE:             control.name = "Hue"; break;
                case DC1394_FEATURE_SATURATION:      control.name = "Saturation"; break;
                case DC1394_FEATURE_GAMMA:           control.name = "Gamma"; break;

                case DC1394_FEATURE_SHUTTER:
                    control.name = "Shutter";
                    control.is_exposure = true;
                    control.is_duration = true;
                    // No way to check the unit using IIDC API, but PGR Firefly MV (FMVU-03MTM) and Chameleon3 (CM3-U3-13S2M) both use seconds
                    control.duration_unit = 1s;
                    break;

                case DC1394_FEATURE_GAIN:            control.name = "Gain"; break;
                case DC1394_FEATURE_IRIS:            control.name = "Iris"; break;
                case DC1394_FEATURE_FOCUS:           control.name = "Focus"; break;
                case DC1394_FEATURE_TEMPERATURE:     control.name = "Temperature"; break;

                // TODO: requires special handling
                // case DC1394_FEATURE_TRIGGER:         control.name = "Trigger"; break;

                // TODO: uncomment when DC1394_FEATURE_TRIGGER is implemented
                // case DC1394_FEATURE_TRIGGER_DELAY:
                //     control.name = "Trigger delay";
                //     control.is_duration = true;
                //     break;

                //TODO: this is in fact a triple of controls
                //case DC1394_FEATURE_WHITE_SHADING:   control.name = "White shading"; break;

                case DC1394_FEATURE_FRAME_RATE:      control.name = "Frame Rate"; break;
                case DC1394_FEATURE_ZOOM:            control.name = "Zoom"; break;
                case DC1394_FEATURE_PAN:             control.name = "Pan"; break;
                case DC1394_FEATURE_TILT:            control.name = "Tilt"; break;
                case DC1394_FEATURE_OPTICAL_FILTER:  control.name = "Optical Filter"; break;
                case DC1394_FEATURE_CAPTURE_SIZE:    control.name = "Capture Size"; break;
                case DC1394_FEATURE_CAPTURE_QUALITY: control.name = "Capture Quality"; break;

                default: continue;
            }

            control.supports_auto = (std::find(feature.modes.modes, feature.modes.modes + feature.modes.num, DC1394_FEATURE_MODE_AUTO)
                                       != feature.modes.modes + feature.modes.num);

            control.readonly = (0 == feature.modes.num &&
                                DC1394_TRUE == feature.readout_capable);

            control.supports_onOff = (DC1394_TRUE == feature.on_off_capable);
            if (control.supports_onOff)
            {
                dc1394switch_t currOnOff;
                IIDC_CHECK << dc1394_feature_get_power(d->camera.get(), feature.id, &currOnOff)
                           << "Get feature on/off";
                control.value_onOff = (DC1394_ON == currOnOff);
            }

            dc1394feature_mode_t currMode;
            IIDC_CHECK << dc1394_feature_get_mode(d->camera.get(), feature.id, &currMode)
                        << "Get feature mode";
            control.value_auto = (DC1394_FEATURE_MODE_AUTO == currMode);

            float absMin, absMax;
            uint32_t rawMin, rawMax;

            d->getRawRange(feature.id, rawMin, rawMax);

            // A feature is "absolute control-capable", if its value can be set using
            // floating-point arguments, not just the integer "raw/driver" values.
            // E.g. SHUTTER can be set in fractional "absolute" values expressed in seconds.
            dc1394bool_t absoluteCapable;
            IIDC_CHECK << dc1394_feature_has_absolute_control(d->camera.get(), feature.id, &absoluteCapable)
                       << "Get feature absolute capability";

            if (DC1394_TRUE == absoluteCapable)
            {
                d->hasAbsoluteControl[feature.id] = true;

                control.decimals = 6;

                IIDC_CHECK << dc1394_feature_set_absolute_control(d->camera.get(), feature.id, DC1394_ON)
                           << "Set feature absolute control";

                d->getAbsoluteRange(feature.id, absMin, absMax);

                if (DC1394_TRUE == feature.readout_capable)
                {
                    float fval;
                    IIDC_CHECK << dc1394_feature_get_absolute_value(d->camera.get(), feature.id, &fval)
                               << "Get feature absolute value";

                    control.value = fval;
                }
                else
                    control.value = fmin;
            }
            else
            {
                d->hasAbsoluteControl[feature.id] = false;

                control.decimals = 0;

                if (DC1394_TRUE == feature.readout_capable)
                {
                    uint32_t rawVal;
                    IIDC_CHECK << dc1394_feature_get_value(d->camera.get(), feature.id, &rawVal)
                               << "Get feature value";

                    control.value = rawVal;
                }
                else
                    control.value = rawMin;
            }

            UpdateRangeAndStep(d->hasAbsoluteControl[feature.id], control, rawMin, rawMax, absMin, absMax);

            if (DC1394_FEATURE_SHUTTER == feature.id)
                d->ctrlShutter = control; // See the comment for ctrlShutter

            controls.push_back(std::move(control));
        }

    return controls;
}


void IIDCImager::clearROI()
{
    setROI(QRect{ 0, 0, d->maxFrameSize.width(), d->maxFrameSize.height() });
}

void IIDCImager::setROI(const QRect &roi)
{
    d->currentROI = d->roiValidator->validate(roi, QRect{ });
    startLive();
}

void IIDCImager::startLive()
{
    restart([this] { return std::make_shared<IIDCImagerWorker>(d->camera.get(), d->currentVidMode, d->currentROI); });
    qDebug() << "Video streaming started successfully";
}

void IIDCImager::Private::setHighestFrameRate(dc1394video_mode_t vidMode)
{
    if (!dc1394_is_video_mode_scalable(vidMode))
    {
        const dc1394framerates_t &f = frameRates[vidMode];

        const dc1394framerate_t *highest = std::max_element(f.framerates, f.framerates + f.num);

        IIDC_CHECK << dc1394_video_set_framerate(camera.get(), *highest)
                   << "Set frame rate";
    }
}

void IIDCImager::Private::updateShutterCtrl()
{
    uint32_t rawMin, rawMax;
    float absMin, absMax;

    getRawRange(DC1394_FEATURE_SHUTTER, rawMin, rawMax);

    const bool hasAbsControl = hasAbsoluteControl[DC1394_FEATURE_SHUTTER];
    if (hasAbsControl)
        getAbsoluteRange(DC1394_FEATURE_SHUTTER, absMin, absMax);

    UpdateRangeAndStep(hasAbsControl, ctrlShutter, rawMin, rawMax, absMin, absMax);
}
