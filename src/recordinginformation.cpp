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

#include "recordinginformation.h"
#include <Qt/strings.h>
#include <QDateTime>
#include "drivers/imager.h"
#include <QJsonDocument>
#include "configuration.h"
#include <QFile>

DPTR_IMPL(RecordingInformation) {
  QDateTime started;
  QString filename;
  QVariantMap properties;
};

RecordingInformation::RecordingInformation(Configuration& configuration, Imager *imager) : dptr(QDateTime::currentDateTime())
{
  d->properties["started"] = d->started.toString(Qt::ISODate);
  d->properties["camera"] = imager->name();
  d->properties["observer"] = configuration.observer();
  d->properties["telescope"] = configuration.telescope();
  QVariantMap camera_settings;
  for(auto setting: imager->controls()) {
    QVariantMap setting_value;
    setting_value["value"] = setting.value;
    static QMap<Imager::Control::Type, QString> types {
      {Imager::Control::Number, "number"},
      {Imager::Control::Combo, "combo"},
      {Imager::Control::Bool, "bool"}
    };
    setting_value["type"] = types[setting.type];
    if(setting.type == Imager::Control::Combo) {
      QVariantMap choices;
      for(auto choice: setting.choices)
	choices[choice.label] = choice.value;
      setting_value["choices"] = choices;
    }
    if(setting.type == Imager::Control::Number && setting.is_duration) {
      setting_value["type"] = "duration";
      QList<QPair<QString, double>> units {
	{"seconds", 1}, {"milliseconds", 1000}, {"microseconds", 1000000}
      };
      for(auto unit: units) {
	setting_value["value_%1"_q % unit.first] = setting.value * setting.duration_unit.count() * unit.second;
      }
    }
    camera_settings[setting.name] = setting_value;
  }
  d->properties["camera-settings"] = camera_settings;
}

void RecordingInformation::set_ended(int total_frames, int width, int height, uint8_t bpp, uint8_t channels)
{
  auto ended =QDateTime::currentDateTime();
  auto elapsed = d->started.secsTo(ended);
  d->properties["ended"] = ended.toString(Qt::ISODate);
  d->properties["total-frames"] = total_frames;
  d->properties["width"] = width;
  d->properties["height"] = height;
  d->properties["bpp"] = bpp;
  d->properties["channels"] = channels;
  d->properties["mean-fps"] = static_cast<double>(total_frames) / static_cast<double>(elapsed);
}

void RecordingInformation::set_base_filename(const QString& filename)
{
  d->filename = filename + ".txt";
}

RecordingInformation::~RecordingInformation()
{
  QFile file(d->filename);
  if(d->filename.isEmpty() || ! file.open(QIODevice::WriteOnly))
    return;
  auto json = QJsonDocument::fromVariant(d->properties);
  file.write(json.toJson(QJsonDocument::Indented));
  file.close();
}
