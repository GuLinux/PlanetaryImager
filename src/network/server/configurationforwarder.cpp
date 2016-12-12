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

#include "configurationforwarder.h"
#include "network/protocol/configurationprotocol.h"

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(ConfigurationForwarder) {
  Configuration::ptr configuration;
  ConfigurationForwarder *q;
  void get(const NetworkPacket::ptr &packet);
  void set(const NetworkPacket::ptr &packet);
  void reset(const NetworkPacket::ptr &packet);
  
  struct ConfigurationFunctions {
    function<QVariant()> get;
    function<void(const QVariant &)> set;
    function<void()> reset;
  };
  QHash<QString, ConfigurationFunctions> names;
};

#define register_conf_function(name, type) d->names[#name] = Private::ConfigurationFunctions{ \
  [this] { return QVariant{ d->configuration->name() }; }, \
  [this](const QVariant &v) { d->configuration->set_ ##name( v.value<type>() ); }, \
  [this] { d->configuration->reset_ ##name(); }, \
};
#define register_conf_function_enum(name, type) d->names[#name] = Private::ConfigurationFunctions{ \
  [this] { return QVariant{ static_cast<int>(d->configuration->name() ) }; }, \
  [this](const QVariant &v) { d->configuration->set_ ##name( static_cast<type>(v.value<int>() ) ); }, \
  [this] { d->configuration->reset_ ##name(); }, \
};

ConfigurationForwarder::ConfigurationForwarder(const Configuration::ptr& configuration, const NetworkDispatcher::ptr& dispatcher) : NetworkReceiver{dispatcher}, dptr(configuration, this)
{
  register_handler(ConfigurationProtocol::Get, bind(&Private::get, d.get(), _1));
  register_handler(ConfigurationProtocol::Set, bind(&Private::set, d.get(), _1));
  register_handler(ConfigurationProtocol::Reset, bind(&Private::reset, d.get(), _1));

  register_conf_function(buffered_output, bool )
  register_conf_function(max_memory_usage, long long )
  
  register_conf_function_enum(recording_limit_type, Configuration::RecordingLimit)
  register_conf_function(recording_seconds_limit, double )
  register_conf_function(recording_frames_limit, long long )
  
  register_conf_function(save_directory, QString)
  register_conf_function(save_file_prefix_suffix_separator, QString)
  register_conf_function(save_file_avail_prefixes, QStringList)
  register_conf_function(save_file_avail_suffixes, QStringList)
  register_conf_function(save_file_prefix, QString)
  register_conf_function(save_file_suffix, QString )
  
  register_conf_function_enum(save_format, Configuration::SaveFormat)
  register_conf_function(video_codec, QString)
  register_conf_function(save_json_info_file, bool)
  register_conf_function(save_info_file, bool)
  
  register_conf_function(observer, QString)
  register_conf_function(telescope, QString)
  
  register_conf_function(timelapse_mode, bool)
  register_conf_function(timelapse_msecs, qlonglong)
}

ConfigurationForwarder::~ConfigurationForwarder()
{
}


void ConfigurationForwarder::Private::get(const NetworkPacket::ptr& packet)
{
  auto name = packet->payloadVariant().toString();
  auto value = names[name].get();
  q->dispatcher()->queue_send(ConfigurationProtocol::encodeGetReply(name, value));
}

void ConfigurationForwarder::Private::reset(const NetworkPacket::ptr& packet)
{
  names[packet->payloadVariant().toString()].reset();
}

void ConfigurationForwarder::Private::set(const NetworkPacket::ptr& packet)
{
  QString name;
  QVariant value;
  ConfigurationProtocol::decodeSet(packet, name, value);
  names[name].set(value);
}
