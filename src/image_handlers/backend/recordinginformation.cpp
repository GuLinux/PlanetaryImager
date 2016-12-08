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
#include "commons/configuration.h"
#include <QFile>

using namespace std;

DPTR_IMPL(RecordingInformation) {
  QDateTime started;
  Writer::ptr writer;
  QVariantMap properties;
};

RecordingInformation::RecordingInformation(const Configuration::ptr & configuration, Imager *imager) : dptr(QDateTime::currentDateTime())
{
  d->properties["started"] = d->started.toString(Qt::ISODate);
  d->properties["camera"] = imager->name();
  d->properties["observer"] = configuration->observer();
  d->properties["telescope"] = configuration->telescope();
  QVariantList controls;
  for(auto control: imager->controls()) {
    controls.push_back(control.asMap());
  }
  d->properties["controls"] = controls;
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

void RecordingInformation::set_writer(const Writer::ptr& writer)
{
  d->writer = writer;
}


RecordingInformation::~RecordingInformation()
{
  if(d->writer)
    d->writer->write(d->properties);
}

namespace {
  class JSONRecordingInformationWriter : public RecordingInformation::Writer {
  public:
    JSONRecordingInformationWriter(const QString &file_base_name);
    virtual void write(const QVariantMap &information);
  private:
    const QString file_base_name;
  };
  
  class TXTRecordingInformationWriter : public RecordingInformation::Writer {
  public:
    TXTRecordingInformationWriter(const QString &file_base_name);
    virtual void write(const QVariantMap &information);
  private:
    const QString file_base_name;
  };
  
  class CompositeRecordingInformationWriter : public RecordingInformation::Writer {
  public:
    CompositeRecordingInformationWriter(const QList<ptr> &writers);
    virtual void write(const QVariantMap &information);
  private:
    const QList<ptr> writers;
  };
  
  template<typename T>void  write_recording_info_to_file(const QString &filename, const QString &extension, const T &data) {
    QFile file(filename + "." + extension);
    if(filename.isEmpty() || ! file.open(QIODevice::WriteOnly))
      return;
    file.write(data);
    file.close();
  }
  
  QList<QPair<QString, QString>> recording_map_to_strings(const QVariantMap &map, const QString &prefix = "") {
    QList<QPair<QString,QString>> result;
    for(auto key: map.keys()) {
      QVariant value = map[key];
      if(value.type() == QVariant::Map) {
        result.append(recording_map_to_strings(value.toMap(), prefix + key + "_"));
        continue;
      }
      if(value.type() == QVariant::List) {
        qDebug() << "to be implemented: QVariant::List to recording info (" << prefix << ", " << key << ")";
        // TODO: handle. Currently not in use
      }
      result.append({prefix + key, value.toString()});
    }
    return result;
  }
}

JSONRecordingInformationWriter::JSONRecordingInformationWriter(const QString &file_base_name) : file_base_name{file_base_name} {
}

void JSONRecordingInformationWriter::write(const QVariantMap &information) {
  auto json = QJsonDocument::fromVariant(information);
  write_recording_info_to_file(file_base_name, "json", json.toJson(QJsonDocument::Indented));
}


TXTRecordingInformationWriter::TXTRecordingInformationWriter(const QString &file_base_name) : file_base_name{file_base_name} {
}

void TXTRecordingInformationWriter::write(const QVariantMap &information) {
  QString variant_map_to_string;
  
  for(auto value: recording_map_to_strings(information))
    variant_map_to_string += "%1: %2\n"_q % value.first % value.second;
  write_recording_info_to_file(file_base_name, "txt", variant_map_to_string.toLatin1());
}


CompositeRecordingInformationWriter::CompositeRecordingInformationWriter(const QList<ptr> &writers) : writers{writers} {
}

void CompositeRecordingInformationWriter::write(const QVariantMap &information) {
  for(auto writer: writers)
    writer->write(information);
}



RecordingInformation::Writer::ptr RecordingInformation::json(const QString& file_base_name)
{
  return make_shared<JSONRecordingInformationWriter>(file_base_name);
}

RecordingInformation::Writer::ptr RecordingInformation::txt(const QString& file_base_name)
{
  return make_shared<TXTRecordingInformationWriter>(file_base_name);
}

RecordingInformation::Writer::ptr RecordingInformation::composite(const QList<RecordingInformation::Writer::ptr> &writers)
{
  return make_shared<CompositeRecordingInformationWriter>(writers);
}

