/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
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

#include "driverprotocol.h"
#include "network/networkpacket.h"
#include <algorithm>
#include <functional>
#include <opencv2/opencv.hpp>
#include "commons/opencv_utils.h"
#include "commons/frame.h"
#include <QJsonDocument>
#include "drivers/driver.h"

using namespace std;
using namespace std::placeholders;
PROTOCOL_NAME_VALUE(Driver, CameraList);
PROTOCOL_NAME_VALUE(Driver, CameraListReply);
PROTOCOL_NAME_VALUE(Driver, ConnectCamera);
PROTOCOL_NAME_VALUE(Driver, GetCameraName);
PROTOCOL_NAME_VALUE(Driver, GetCameraNameReply);
PROTOCOL_NAME_VALUE(Driver, GetProperties);
PROTOCOL_NAME_VALUE(Driver, GetPropertiesReply);
PROTOCOL_NAME_VALUE(Driver, StartLive);
PROTOCOL_NAME_VALUE(Driver, StartLiveReply);
PROTOCOL_NAME_VALUE(Driver, ClearROI);
PROTOCOL_NAME_VALUE(Driver, GetControls);
PROTOCOL_NAME_VALUE(Driver, GetControlsReply);
PROTOCOL_NAME_VALUE(Driver, SendFrame);
PROTOCOL_NAME_VALUE(Driver, SetControl);
PROTOCOL_NAME_VALUE(Driver, SetROI);
PROTOCOL_NAME_VALUE(Driver, signalFPS);
PROTOCOL_NAME_VALUE(Driver, signalTemperature);
PROTOCOL_NAME_VALUE(Driver, signalControlChanged);
PROTOCOL_NAME_VALUE(Driver, signalDisconnected);
PROTOCOL_NAME_VALUE(Driver, signalCameraConnected);
PROTOCOL_NAME_VALUE(Driver, CloseCamera);

namespace {
  QVariant control2variant(const Imager::Control &control) {
    QVariantList choices;
    transform(begin(control.choices), end(control.choices), back_inserter(choices), [](const Imager::Control::Choice c){
      return QVariantMap{ {"label", c.label}, {"val", c.value} };
    });
    return QVariantMap {
      {"id",  control.id},
      {"name",  control.name},
      {"val",  control.value},
      {"def",  control.default_value},
      {"type",  static_cast<int>(control.type)},
      {"min",  control.range.min},
      {"max",  control.range.max},
      {"step",  control.range.step},
      {"choices", choices},
      {"decimals", control.decimals},
      {"is_duration", control.is_duration},
      {"has_auto", control.supports_auto},
      {"is_auto", control.value_auto},
      {"ro", control.readonly},
      {"duration_unit", control.duration_unit.count()},
    };
  }

  Imager::Control variant2control(const QVariant &variant) {
    QVariantMap ctrl = variant.toMap();
    Imager::Control::Choices choices;
    for(auto c: ctrl["choices"].toList()) {
      QVariantMap choice = c.toMap();
      choices.push_back({choice["label"].toString(), choice["val"]});
      qDebug() << "Choice: " << Imager::Control::Choice{choice["label"].toString(), choice["val"]};
    }
    return {
      ctrl["id"].toLongLong(),
      ctrl["name"].toString(),
      static_cast<Imager::Control::Type>(ctrl["type"].toInt()),
      ctrl["val"],
      ctrl["def"],
      { ctrl["min"], ctrl["max"], ctrl["step"] },
      choices,
      static_cast<qint16>(ctrl["decimals"].toInt()),
      ctrl["is_duration"].toBool(),
      ctrl["has_auto"].toBool(),
      ctrl["is_auto"].toBool(),
      ctrl["ro"].toBool(),
      std::chrono::duration<double>{ctrl["duration_unit"].toDouble()},
    };
  }
  static NetworkProtocol::FormatParameters format_parameters;
  static vector<int> opencv_encode_parameters;
}

void DriverProtocol::setFormatParameters(const FormatParameters& parameters)
{
  format_parameters = parameters;
  opencv_encode_parameters.clear();
  if(parameters.format == Configuration::Network_JPEG) {
    opencv_encode_parameters = {cv::IMWRITE_JPEG_QUALITY  , parameters.jpegQuality };
  }
  if(parameters.format == Configuration::Network_RAW) {
    opencv_encode_parameters = {cv::IMWRITE_PXM_BINARY , 1 };
  }
}

bool DriverProtocol::isForwardingEnabled()
{
  return format_parameters.format != Configuration::Network_NoImage;
}


NetworkPacketPtr DriverProtocol::sendCameraListReply(const QList<CameraPtr>& cameras)
{
  QVariantList v_cameras;
  transform(begin(cameras), end(cameras), back_inserter(v_cameras), [](const CameraPtr &c){
    QVariantMap p;
    p["n"] = c->name();
    p["a"] = reinterpret_cast<qlonglong>(c.get());
    return p;
  });
  return packetCameraListReply() << v_cameras;
}

void DriverProtocol::decode(QList<CameraPtr>& cameras, const NetworkPacketPtr& packet, const CameraFactory& factory)
{
  auto v_cameras = packet->payloadVariant().toList();
  transform(begin(v_cameras), end(v_cameras), back_inserter(cameras), [&](const QVariant &v) { return factory(v.toMap()["n"].toString(), v.toMap()["a"].toLongLong()); });
}



NetworkPacketPtr DriverProtocol::sendGetPropertiesReply(const Imager::Properties& properties)
{
  QVariantList l;
  QVariantList caps;
  transform(begin(properties.properties), end(properties.properties), back_inserter(l), [](const Imager::Properties::Property &p) {
    return QVariantMap {
      {"name", p.name},
      {"value", p.value},
      {"display_name", p.display_name},
      {"display_value", p.display_value},
    };
  });
  transform(begin(properties.capabilities), end(properties.capabilities), back_inserter(caps), [](const Imager::Capability &c) { return static_cast<int>(c); } );
  return packetGetPropertiesReply() << QVariantMap{ {"properties", l},  {"capabilities", caps} };
}


void DriverProtocol::decode(Imager::Properties& properties, const NetworkPacketPtr& packet)
{
  properties.capabilities.clear();
  properties.properties.clear();
  QVariantMap packetMap = packet->payloadVariant().toMap();
  QVariantList p = packetMap["properties"].toList();
  transform(begin(p), end(p), back_inserter(properties.properties), [](const QVariant &v){
    auto m = v.toMap();
    return Imager::Properties::Property{
      m["name"].toString(),
      m["value"].toString(),
      m["display_name"].toString(),
      m["display_value"].toString(),
    };
  });
  for(auto v: packetMap["capabilities"].toList() )
    properties.capabilities.insert( static_cast<Imager::Capability>(v.toInt()) );
}

NetworkPacketPtr DriverProtocol::sendGetControlsReply(const Imager::Controls& controls)
{
  QVariantList v;
  transform(begin(controls), end(controls), back_inserter(v), bind(control2variant, _1));
  //qDebug().noquote().nospace() << "controls encoded: " << QJsonDocument::fromVariant(v).toJson(QJsonDocument::Compact);
  return packetGetControlsReply() << v;
//   return packetGetControlsReply() << QJsonDocument::fromVariant(v).toBinaryData();
}


void DriverProtocol::decode(Imager::Controls& controls, const NetworkPacketPtr& packet)
{
  controls.clear();
//   QVariantList variant_controls = QJsonDocument::fromBinaryData(packet->payload()).toVariant().toList();
  QVariantList variant_controls = packet->payloadVariant().toList();
  //qDebug().noquote().nospace() << "controls to decode: " << QJsonDocument::fromVariant(variant_controls).toJson(QJsonDocument::Compact);
  transform(begin(variant_controls), end(variant_controls), back_inserter(controls), bind(variant2control, _1));
}

NetworkPacketPtr DriverProtocol::sendFrame(FrameConstPtr frame)
{
  if(format_parameters.format == Configuration::Network_NoImage)
    return {};

  vector<uint8_t> data;
  QByteArray image;
  string extension;


  if(format_parameters.format == Configuration::Network_JPEG) {
    extension = ".jpg";
  } else if(format_parameters.format == Configuration::Network_RAW) {
    extension = frame->channels() == 1 ? ".pgm" : ".ppm";
  };
  cv::Mat cv_image = frame->mat();
  if(
    ( (format_parameters.force8bit && format_parameters.format == Configuration::Network_RAW) || format_parameters.format ==Configuration::Network_JPEG )
    && frame->bpp() > 8
  ) {
    cv_image.convertTo(cv_image, frame->channels() == 1 ? CV_8UC1 : CV_8UC3, 1./256.);
  }
  cv::imencode(extension, cv_image, data, opencv_encode_parameters );
  image.resize(data.size());
  move(begin(data), end(data), begin(image));
  if(format_parameters.compression && format_parameters.format == Configuration::Network_RAW) {
    image = qCompress(image, 1);
  }
  //qDebug() << "FRAME data size: " << image.size() << ", bpp: " << frame->bpp() << ", res: " << frame->resolution() << ", channels: " << frame->channels();
  auto packet = packetSendFrame();
  packet->movePayload(std::move(image));
  return packet;
}

FramePtr DriverProtocol::decodeFrame(const NetworkPacketPtr& packet)
{
  QByteArray image = packet->payload();
  if(format_parameters.compression && format_parameters.format == Configuration::Network_RAW) {
    image = qUncompress(image);
  }
  vector<uint8_t> data(image.size());
  move(begin(image), end(image), begin(data));
  auto mat = cv::imdecode(data, cv::IMREAD_UNCHANGED);
  auto frame = make_shared<Frame>(mat.channels() == 1 ? Frame::Mono : Frame::BGR, mat, Frame::LittleEndian);
  //qDebug() << "FRAME data size: " << image.size() << ", bpp: " << frame->bpp() << ", res: " << frame->resolution() << ", channels: " << frame->channels();

  return frame;
}

NetworkPacketPtr DriverProtocol::setControl(const Imager::Control& control)
{
  return packetSetControl() << control2variant(control);
}

NetworkPacketPtr DriverProtocol::controlChanged(const Imager::Control& control)
{
  return packetsignalControlChanged() << control2variant(control);
}

Imager::Control DriverProtocol::decodeControl(const NetworkPacketPtr &packet) {
  return variant2control(packet->payloadVariant());
}

DriverProtocol::DriverStatus DriverProtocol::decodeStatus(const NetworkPacketPtr& packet)
{
  return { packet->payloadVariant().toMap()["imager_running"].toBool() };
}

void DriverProtocol::encodeStatus(const DriverProtocol::DriverStatus& status, QVariantMap& data)
{
  data["imager_running"] = status.imager_running;
}

