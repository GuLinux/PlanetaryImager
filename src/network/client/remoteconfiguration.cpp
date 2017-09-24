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

#include "remoteconfiguration.h"
#include "Qt/qt_strings_helper.h"
#include "network/protocol/configurationprotocol.h"

using namespace std;

DPTR_IMPL(RemoteConfiguration) {
  RemoteConfiguration *q;
  template<typename T> T get(const QString &name) const;
  template<typename T> void set(const QString &name, const T &value);
  void reset(const QString &name);
  QVariantMap values;
};

RemoteConfiguration::RemoteConfiguration(const NetworkDispatcher::ptr& dispatcher) : NetworkReceiver{dispatcher}, dptr(this)
{
  register_handler(ConfigurationProtocol::GetReply, [this](const NetworkPacket::ptr &packet) {
    QString name;
    QVariant value;
    ConfigurationProtocol::decodeGetReply(packet, name, value);
    d->values[name] = value;
  });
}

RemoteConfiguration::~RemoteConfiguration()
{
}
template<typename T> T RemoteConfiguration::Private::get(const QString &name) const
{
  q->dispatcher()->send(ConfigurationProtocol::get(name));
  q->wait_for_processed(ConfigurationProtocol::GetReply);
  return values[name].value<T>();
}

template<typename T> void RemoteConfiguration::Private::set(const QString &name, const T &value)
{
  q->dispatcher()->send(ConfigurationProtocol::set(name, QVariant{value}));
}

void RemoteConfiguration::Private::reset(const QString &name)
{
  q->dispatcher()->send(ConfigurationProtocol::reset(name));
}


#define define_setting_get(name, type) type RemoteConfiguration::name() const { return d->get<type>(#name); }
#define define_setting_enum_get(name, type) type RemoteConfiguration::name() const { return static_cast<type>( d->get<int>(#name) ); }
#define define_setting_set(name, type) void RemoteConfiguration::set_ ##name(const type &value) { d->set(#name, value); }
#define define_setting_enum_set(name, type) void RemoteConfiguration::set_ ##name(const type &value) { d->set( #name, static_cast<int>(value)); }
#define define_setting_reset(name) void RemoteConfiguration::reset_ ##name() { d->reset(#name); }

#define define_setting(name, type) define_setting_get(name, type)  define_setting_set(name, type) define_setting_reset(name)
#define define_setting_enum(name, type) define_setting_enum_get(name, type)  define_setting_enum_set(name, type) define_setting_reset(name)

define_setting(buffered_output, bool )
define_setting(max_memory_usage, long long )

define_setting_enum(recording_limit_type, Configuration::RecordingLimit)
define_setting(recording_seconds_limit, double )
define_setting(recording_frames_limit, long long )

define_setting(save_directory, QString)
define_setting(save_file_prefix_suffix_separator, QString)
define_setting(save_file_avail_prefixes, QStringList)
define_setting(save_file_avail_suffixes, QStringList)
define_setting(save_file_prefix, QString)
define_setting(save_file_suffix, QString )

define_setting_enum(save_format, Configuration::SaveFormat)
define_setting(video_codec, QString)
define_setting(save_json_info_file, bool)
define_setting(save_info_file, bool)

define_setting(observer, QString)
define_setting(telescope, QString)

define_setting(timelapse_mode, bool)
define_setting(timelapse_msecs, qlonglong)
define_setting(recording_pause_stops_timer, bool)
