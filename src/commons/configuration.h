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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include "dptr.h"
#include <QString>

class QSettings;
class Configuration
{
public:
    Configuration();
    ~Configuration();

    // Try avoiding directly accessing QSettings
    [[deprecated]] std::shared_ptr<QSettings> qSettings() const;

#define declare_setting(name, type) \
    void set_## name(const type &value); \
    void reset_## name(); \
    type name() const;

    declare_setting(dock_status, QByteArray )
    declare_setting(main_window_geometry, QByteArray )
    declare_setting(max_memory_usage, long long )
    declare_setting(buffered_output, bool )
    declare_setting(max_display_fps, int )
    declare_setting(limit_fps, bool)
    declare_setting(max_display_fps_recording, int )
    declare_setting(limit_fps_recording, bool)
    declare_setting(debayer, bool)
    declare_setting(opengl, bool)
    
    enum RecordingLimit { Infinite = 0, FramesNumber = 1, Duration = 2, FileSize = 3};
    declare_setting(recording_limit_type, RecordingLimit)
    declare_setting(recording_seconds_limit, double )
    declare_setting(recording_frames_limit, long long )
    
    declare_setting(save_directory, QString)
    declare_setting(save_file_prefix_suffix_separator, QString)
    declare_setting(save_file_avail_prefixes, QStringList)
    declare_setting(save_file_avail_suffixes, QStringList)
    declare_setting(save_file_prefix, QString)
    declare_setting(save_file_suffix, QString )
    declare_setting(observer, QString)
    declare_setting(telescope, QString)

    enum EdgeAlgorithm { Sobel, Canny };
    declare_setting(edge_algorithm, EdgeAlgorithm)
    declare_setting(sobel_kernel, int)
    declare_setting(sobel_blur_size, int)
    declare_setting(sobel_scale, double)
    declare_setting(sobel_delta, double)
    declare_setting(canny_low_threshold, double)
    declare_setting(canny_threshold_ratio, double)
    declare_setting(canny_kernel_size, int)
    declare_setting(canny_blur_size, int)

    void resetCannyAdvancedSettings();
    void resetSobelAdvancedSettings();

    enum SaveFormat { SER, Video };
    declare_setting(save_format, SaveFormat)
    declare_setting(video_codec, QString)
    declare_setting(save_json_info_file, bool)
    declare_setting(save_info_file, bool)
    declare_setting(widgets_setup_first_run, bool)
    declare_setting(histogram_bins, int)
    declare_setting(histogram_logarithmic, bool)
    declare_setting(histogram_disable_on_recording, bool)
    declare_setting(histogram_timeout, long long)
    declare_setting(histogram_timeout_recording, long long)
    
    QString savefile() const;

private:
    DPTR
};


#endif // CONFIGURATION_H
