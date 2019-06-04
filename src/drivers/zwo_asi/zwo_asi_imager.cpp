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

#include "zwo_asi_imager.h"
#include "drivers/imagerthread.h"
#include <stringbuilder.h>
#include <QObject>
#include <set>
#include <QThread>
#include "commons/fps_counter.h"
#include <QRect>
#include <atomic>
#include <ratio>
#include "Qt/qt_strings_helper.h"
#include "commons/utils.h"
#include "zwoexception.h"
#include "asiimagingworker.h"
#include <QTimer>
#include "asicontrol.h"
#include <QCoreApplication>
#include "drivers/roi.h"
#include "c++/stlutils.h"

using namespace std;
using namespace std::chrono_literals;
using namespace GuLinux;


namespace {
const int64_t ImgTypeControlID = 10000;
const int64_t BinControlID = 10001;
}


DPTR_IMPL(ZWO_ASI_Imager) {
    ASI_CAMERA_INFO info;
    ZWO_ASI_Imager *q;

    Properties properties;

    ASIControl::vector controls;
    ASIControlPtr temperature_control;
    
    weak_ptr<ASIImagingWorker> worker;
    ROIValidator::ptr roi_validator;
    QRect maxROI(int bin) const;
    void restart_worker(int bin, const QRect &roi, ASI_IMG_TYPE format);
    void update_worker_exposure_timeout();
    ASI_IMG_TYPE format() { return worker.expired() ? ASI_IMG_END : worker.lock()->format(); }
    int bin() { return worker.expired() ? -1 : worker.lock()->bin(); }
    QRect roi() { return worker.expired() ? QRect{} : worker.lock()->roi(); }
};


ZWO_ASI_Imager::ZWO_ASI_Imager(const ASI_CAMERA_INFO &info, const ImageHandler::ptr &imageHandler) : Imager{imageHandler}, dptr(info, this)
{
    d->roi_validator = make_shared<ROIValidator>(initializer_list<ROIValidator::Rule>{
      ROIValidator::x_multiple(2),
      ROIValidator::y_multiple(2),
      ROIValidator::width_multiple(8),
      ROIValidator::height_multiple(2),
      [=](QRect &roi) {
        if(info.IsUSB3Camera == ASI_FALSE && QString{info.Name}.contains("120")) {
          ROIValidator::area_multiple(1024, 0, 2, QRect{0, 0, static_cast<int>(info.MaxWidth), static_cast<int>(info.MaxHeight)})(roi);
          qDebug() << "Using ASI 120MM rect roi definition: " << roi;
        }
      }
    });
    d->properties.set_resolution_pixelsize({static_cast<int>(info.MaxWidth), static_cast<int>(info.MaxHeight)}, info.PixelSize, info.PixelSize);
    d->properties << Properties::Property{"Camera Speed", info.IsUSB3Camera ? "USB3" : "USB2"}
                  << Properties::Property{"Host Speed", info.IsUSB3Host ? "USB3" : "USB2"}
                  << Properties::Property{"Camera Type", info.IsColorCam ? "colour" : "mono"};
    if(info.IsColorCam) {
        static map<ASI_BAYER_PATTERN, QString> patterns {
            {ASI_BAYER_RG, "RGGB"}, {ASI_BAYER_BG, "BGGR"}, {ASI_BAYER_GR, "GRBG"}, {ASI_BAYER_GB, "GBRG"}
        };
        d->properties << Properties::Property{"Bayer pattern", patterns[info.BayerPattern]};
    }
    
    d->properties << Properties::Property{"ElecPerADU", info.ElecPerADU};
    d->properties << Properties::Property{"ASI SDK Version", ASI_SDK_VERSION};
    d->properties << LiveStream << ROI << Temperature;
    ASI_CHECK << ASIOpenCamera(info.CameraID) << "Open Camera";
    ASI_CHECK << ASIInitCamera(info.CameraID) << "Init Camera";
    connect(this, &Imager::exposure_changed, this, bind(&Private::update_worker_exposure_timeout, d.get()));
}

ZWO_ASI_Imager::~ZWO_ASI_Imager()
{
}

void ZWO_ASI_Imager::destroy() {
    Imager::destroy();
    d->worker.reset();
    ASI_CHECK << ASICloseCamera(d->info.CameraID) << "Close Camera";
}

void ZWO_ASI_Imager::readTemperature() {
    qDebug() << "Refreshing ASI_TEMPERATURE if found..";
    if(d->temperature_control)
        push_job_on_thread([=]{
           emit temperature(d->temperature_control->reload().control().value.toDouble());
        });
}

Imager::Properties ZWO_ASI_Imager::properties() const
{
    return d->properties;
}

QString ZWO_ASI_Imager::name() const
{
    return d->info.Name;
}

void ZWO_ASI_Imager::Private::update_worker_exposure_timeout()
{
  if(!worker.expired())
    worker.lock()->calc_exposure_timeout();
}


Imager::Controls ZWO_ASI_Imager::controls() const
{
    Controls controls;
    int controls_number;
    ASI_CHECK << ASIGetNumOfControls(d->info.CameraID, &controls_number) << "Get controls";
    d->controls = ASIControl::vector(controls_number);

    for(int control_index = 0; control_index < controls_number; control_index++) {
      auto control = make_shared<ASIControl>(control_index, d->info.CameraID);
      d->controls[control_index] = control;
      if(control->caps.ControlType == ASI_TEMPERATURE)
        d->temperature_control = control;
      else
        controls.push_back(control->control());
    }

    static map<ASI_IMG_TYPE, QString> format_names {
        {ASI_IMG_RAW8, "Raw 8bit"},
        {ASI_IMG_RGB24, "RGB24"},
        {ASI_IMG_RAW16, "RAW 16bit"},
        {ASI_IMG_Y8, "Y8 (Monochrome)"},
    };
    auto imageFormat = Control{ImgTypeControlID, "Image Format", Control::Combo}.set_value_enum(d->format());
    int i = 0;
    while(d->info.SupportedVideoFormat[i] != ASI_IMG_END && i < 8) {
        auto format = d->info.SupportedVideoFormat[i];
        qDebug() << "supported format: " << format << ": " << format_names[format];
        imageFormat.add_choice_enum(format_names[format], format);
        ++i;
    }
    controls.push_front(imageFormat);

    auto bin = Control{BinControlID, "Bin", Control::Combo}.set_value(d->bin());
    i = 0;
    while(d->info.SupportedBins[i] != 0) {
        auto bin_value = d->info.SupportedBins[i++];
        bin.add_choice("%1x%1"_q % bin_value, bin_value);
    }
    controls.push_front(bin);
    return controls;
}


void ZWO_ASI_Imager::setControl(const Control& control)
{
  LOG_F_SCOPE
  if(control.id == ImgTypeControlID) {
    d->restart_worker(d->bin(), d->roi(), control.get_value_enum<ASI_IMG_TYPE>());
    emit changed(control);
    return;
  }
  if(control.id == BinControlID) {
    auto bin =control.get_value<int>();
    d->restart_worker(bin, d->maxROI(bin), d->format());
    emit changed(control);
    return;
  }
  auto camera_control_it = find_if(d->controls.begin(), d->controls.end(),
			  [&](const ASIControlPtr &c){ return c->caps.ControlType == static_cast<ASI_CONTROL_TYPE>(control.id); });
  if(camera_control_it != d->controls.end()) {
    auto camera_control = *camera_control_it;
    qDebug() << "Changing control " << camera_control->control();
    wait_for(push_job_on_thread([=]{
      camera_control->set(control.get_value<qlonglong>(), control.value_auto);
      qDebug() << "Changed control " << camera_control->control();
      emit changed(*camera_control);
    }));
  }
}




void ZWO_ASI_Imager::Private::restart_worker(int bin, const QRect& roi, ASI_IMG_TYPE format)
{
  auto factory = [=] {
    auto worker = make_shared<ASIImagingWorker>(roi, bin, info, format);
    this->worker = worker;
    return worker;
  };
  worker.reset();
  q->restart(factory);
}



void ZWO_ASI_Imager::startLive()
{
    LOG_F_SCOPE
    d->restart_worker(1, d->maxROI(1), d->info.SupportedVideoFormat[0]);
    qDebug() << "Live started correctly";
}


void ZWO_ASI_Imager::clearROI()
{
  d->restart_worker(d->bin(), d->maxROI(d->bin()), d->format());
}

void ZWO_ASI_Imager::setROI(const QRect& roi)
{
  auto currentROI = d->roi();
  auto flipControl = find_if(d->controls.begin(), d->controls.end(), [](const auto &c) { return c->caps.ControlType == ASI_FLIP; });
  bool hflip = false, vflip = false;
  if( flipControl!= d->controls.end()) {
    auto flipStatus = (*flipControl)->value;
    hflip = (flipStatus == ASI_FLIP_HORIZ || flipStatus == ASI_FLIP_BOTH);
    vflip = (flipStatus == ASI_FLIP_VERT || flipStatus == ASI_FLIP_BOTH);
  }
  auto flip = [this, hflip, vflip] (const QRect &r) { return ROIValidator::flipped(r, hflip, vflip, d->maxROI(d->bin())); };
  QRect newROI = d->roi_validator->validate(roi, flip(currentROI));
  d->restart_worker(d->bin(), flip(newROI), d->format());
}

QRect ZWO_ASI_Imager::Private::maxROI(int bin) const
{
    return roi_validator->validate({0, 0, static_cast<int>(info.MaxWidth) / bin, static_cast<int>(info.MaxHeight) / bin}, QRect{});
}


#include "zwo_asi_imager.moc"

