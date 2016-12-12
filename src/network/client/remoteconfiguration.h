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

#ifndef REMOTECONFIGURATION_H
#define REMOTECONFIGURATION_H

#include "commons/configuration.h"
#include "network/networkdispatcher.h"
#include "c++/dptr.h"

class RemoteConfiguration : public Configuration, public NetworkReceiver
{
Q_OBJECT
public:
  RemoteConfiguration(const NetworkDispatcher::ptr &dispatcher);
  ~RemoteConfiguration();
  
  declare_setting(buffered_output, bool )
  declare_setting(max_memory_usage, long long )
  
  declare_setting(recording_limit_type, RecordingLimit)
  declare_setting(recording_seconds_limit, double )
  declare_setting(recording_frames_limit, long long )
  
  declare_setting(save_directory, QString)
  declare_setting(save_file_prefix_suffix_separator, QString)
  declare_setting(save_file_avail_prefixes, QStringList)
  declare_setting(save_file_avail_suffixes, QStringList)
  declare_setting(save_file_prefix, QString)
  declare_setting(save_file_suffix, QString )
  
  declare_setting(save_format, SaveFormat)
  declare_setting(video_codec, QString)
  declare_setting(save_json_info_file, bool)
  declare_setting(save_info_file, bool)
  
  declare_setting(observer, QString)
  declare_setting(telescope, QString)
  declare_setting(timelapse_mode, bool)
  declare_setting(timelapse_msecs, qlonglong)
private:
    DPTR
};

#endif // REMOTECONFIGURATION_H
