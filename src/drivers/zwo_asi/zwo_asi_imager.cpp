/*
 * Copyright (C) 2016 Marco Gulino (marco AT gulinux.net)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include "zwo_asi_imager.h"
#include "drivers/imagerthread.h"
#include <stringbuilder.h>
#include <QObject>
#include <QThread>
#include <fps_counter.h>
#include <QRect>
#include <atomic>
#include "Qt/strings.h"
#include "utils.h"
#include "zwoexception.h"
#include "asiimagingworker.h"

using namespace std;
using namespace GuLinux;


namespace {
const int64_t ImageTypeSettingId = 10000;
const int64_t BinSettingId = 10001;

struct ASIControl {
  typedef std::shared_ptr<ASIControl> ptr;
  typedef std::vector<ptr> vector;
  int index;
  int camera_id;
  ASIControl(int index, int camera_id);
  ASI_CONTROL_CAPS caps;
  long value;
  bool is_auto;

  Imager::Setting setting() const;
  operator Imager::Setting() const;
  ASIControl &reload();
  ASIControl &set(long new_value, bool is_auto);
};
}

ASIControl::ASIControl(int index, int camera_id) : index{index}, camera_id{camera_id}
{
  ASI_CHECK << ASIGetControlCaps(camera_id, index, &caps) << "Get control caps";
}

Imager::Setting ASIControl::setting() const
{
  return *this;
}

ASIControl &ASIControl::reload()
{
  ASI_BOOL is_auto;
  ASI_CHECK << ASIGetControlValue(camera_id, caps.ControlType, &value, &is_auto)
            << (stringbuilder() << "Get control value: " << caps.Name);
  this->is_auto = static_cast<bool>(is_auto);
  return *this;
}

ASIControl &ASIControl::set(long new_value, bool is_auto)
{
  ASI_CHECK << ASISetControlValue(camera_id, caps.ControlType, new_value, static_cast<ASI_BOOL>(is_auto))
            << (stringbuilder() << "Set new control value: " << caps.Name << " to " << new_value << " (auto: " << is_auto << ")");
  reload();
  return *this;
}

ASIControl::operator Imager::Setting() const
{
  return {
    static_cast<int64_t>(caps.ControlType),
    caps.Description,
    static_cast<double>(caps.MinValue),
    static_cast<double>(caps.MaxValue),
    1.0,
    static_cast<double>(value),
    static_cast<double>(caps.DefaultValue),
  };
}


DPTR_IMPL(ZWO_ASI_Imager) {
    ASI_CAMERA_INFO info;
    ImageHandlerPtr imageHandler;
    ZWO_ASI_Imager *q;

    Chip chip;

    ASIControl::vector controls;
    
    ImagerThread::ptr imager_thread;
    shared_ptr<ASIImagingWorker> worker;
    QRect maxROI(int bin) const;
    void start_thread(int bin, const QRect& roi, ASI_IMG_TYPE format);
};

ZWO_ASI_Imager::ZWO_ASI_Imager(const ASI_CAMERA_INFO &info, const ImageHandlerPtr &imageHandler) : dptr(info, imageHandler, this)
{
    d->chip.xres = info.MaxWidth;
    d->chip.yres = info.MaxHeight;
    d->chip.pixelheight = info.PixelSize;
    d->chip.pixelwidth = info.PixelSize;
    d->chip.height = info.MaxHeight * info.PixelSize / 1000;
    d->chip.width = info.MaxWidth * info.PixelSize / 1000;
    d->chip.properties.push_back( {"Camera Speed", info.IsUSB3Camera ? "USB3" : "USB2"});
    d->chip.properties.push_back( {"Host Speed", info.IsUSB3Host ? "USB3" : "USB2"});
    ASI_CHECK << ASIOpenCamera(info.CameraID) << "Open Camera";
}

ZWO_ASI_Imager::~ZWO_ASI_Imager()
{
    stopLive();
    ASI_CHECK << ASICloseCamera(d->info.CameraID) << "Close Camera";
}

Imager::Chip ZWO_ASI_Imager::chip() const
{
    return d->chip;
}

QString ZWO_ASI_Imager::name() const
{
    return d->info.Name;
}

void ZWO_ASI_Imager::setSetting(const Setting& setting)
{
    qDebug() << __PRETTY_FUNCTION__;
    if(setting.id == ImageTypeSettingId) {
        d->start_thread(d->worker->bin(), d->worker->roi(), static_cast<ASI_IMG_TYPE>(setting.value));
        emit changed(setting);
        return;
    }
    if(setting.id == BinSettingId) {
        auto bin = static_cast<int>(setting.value);
        d->start_thread(bin, d->maxROI(bin), d->worker->format());
        emit changed(setting);
        return;
    }
    
    auto control = *find_if(d->controls.begin(), d->controls.end(),
                           [&](const ASIControl::ptr &c){ return c->caps.ControlType == static_cast<ASI_CONTROL_TYPE>(setting.id); });
    qDebug() << "Changing setting " << control->setting();
    control->set(static_cast<long>(setting.value), false);
    qDebug() << "Changed setting " << control->setting();
    emit changed(*control);
}

void ZWO_ASI_Imager::Private::start_thread(int bin, const QRect& roi, ASI_IMG_TYPE format)
{
    worker.reset();
    imager_thread.reset();
    worker = make_shared<ASIImagingWorker>(roi, bin, info, format);
    imager_thread = make_shared<ImagerThread>(worker, q, imageHandler);
    imager_thread->start();
}


Imager::Settings ZWO_ASI_Imager::settings() const
{
    qDebug() << __PRETTY_FUNCTION__;
    Settings settings;

    int settings_number;
    ASI_CHECK << ASIGetNumOfControls(d->info.CameraID, &settings_number) << "Get controls";
    d->controls = ASIControl::vector(settings_number);

    for(int setting_index = 0; setting_index < settings_number; setting_index++) {
      d->controls[setting_index] = make_shared<ASIControl>(setting_index, d->info.CameraID);
      settings.push_back(*d->controls[setting_index]);
    }

    static map<ASI_IMG_TYPE, QString> format_names {
        {ASI_IMG_RAW8, "Raw 8bit"},
        {ASI_IMG_RGB24, "RGB24"},
        {ASI_IMG_RAW16, "RAW 16bit"},
        {ASI_IMG_Y8, "Y8"},
    };
    Imager::Setting imageFormat {ImageTypeSettingId, "Image Format", 0., 0., 1., static_cast<double>(d->worker->format()), 0., Setting::Combo};
    int i = 0;
    while(d->info.SupportedVideoFormat[i] != ASI_IMG_END && i < 8) {
        auto format = d->info.SupportedVideoFormat[i];
        qDebug() << "supported format: " << format << ": " << format_names[format];
        imageFormat.choices.push_back( {format_names[format], static_cast<double>(format)});
        ++i;
    }
    imageFormat.max = i-1;

    settings.push_back(imageFormat);

    Imager::Setting bin {BinSettingId, "Bin", 0., 0., 1., static_cast<double>(d->worker->bin()), 1., Setting::Combo};
    i = 0;
    while(d->info.SupportedBins[i] != 0) {
        auto bin_value = d->info.SupportedBins[i++];
        bin.choices.push_back( {"%1x%1"_q % static_cast<double>(bin_value), static_cast<double>(bin_value) } );
    }
    bin.max = i-1;
    settings.push_back(bin);
    return settings;
}

void ZWO_ASI_Imager::startLive()
{
    LOG_F_SCOPE
    d->start_thread(1, d->maxROI(1), d->info.SupportedVideoFormat[0]);
    qDebug() << "Live started correctly";
}

void ZWO_ASI_Imager::stopLive()
{
    d->imager_thread.reset();
}


bool ZWO_ASI_Imager::supportsROI()
{
    return true; // TODO: detection?
}

void ZWO_ASI_Imager::clearROI()
{
    d->start_thread(d->worker->bin(), d->maxROI(d->worker->bin()), d->worker->format());
}

void ZWO_ASI_Imager::setROI(const QRect& roi)
{
    d->start_thread(d->worker->bin(), roi, d->worker->format());
}

QRect ZWO_ASI_Imager::Private::maxROI(int bin) const
{
    return {0, 0, static_cast<int>(info.MaxWidth) / bin, static_cast<int>(info.MaxHeight) / bin};
}


#include "zwo_asi_imager.moc"

