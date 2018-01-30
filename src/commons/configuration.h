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
#include <QObject>

class QSettings;
class Configuration : public QObject
{
  Q_OBJECT
public:
    Configuration();
    ~Configuration();
#define declare_setting(name, type) \
    virtual void set_## name(const type &value); \
    virtual void reset_## name(); \
    virtual type name() const;

    declare_setting(dock_status, QByteArray )
    declare_setting(main_window_geometry, QByteArray )
    declare_setting(max_display_fps, int )
    declare_setting(max_display_fps_recording, int )
    declare_setting(limit_fps_recording, bool)
    declare_setting(debayer, bool)
    declare_setting(limit_fps, bool)
    declare_setting(opengl, bool)
    
    declare_setting(buffered_output, bool )
    declare_setting(max_memory_usage, long long )
    
    enum RecordingLimit { Infinite=0, FramesNumber=1, Duration=2, FileSize=3};
    declare_setting(recording_limit_type, RecordingLimit)
    declare_setting(recording_seconds_limit, double )
    declare_setting(recording_frames_limit, long long )
    
    declare_setting(save_directory, QString)
    declare_setting(save_file_prefix_suffix_separator, QString)
    declare_setting(save_file_avail_prefixes, QStringList)
    declare_setting(save_file_avail_suffixes, QStringList)
    declare_setting(save_file_prefix, QString)
    declare_setting(save_file_suffix, QString )

    enum SaveFormat { SER=0, Video=1, PNG=2, FITS=3 };
    declare_setting(save_format, SaveFormat)
    declare_setting(video_codec, QString)
    declare_setting(save_json_info_file, bool)
    declare_setting(save_info_file, bool)

    declare_setting(observer, QString)
    declare_setting(telescope, QString)

    enum EdgeAlgorithm { Sobel=0, Canny=1};
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

    declare_setting(widgets_setup_first_run, bool)
    declare_setting(histogram_bins, int)
    declare_setting(histogram_logarithmic, bool)
    declare_setting(histogram_disable_on_recording, bool)
    declare_setting(histogram_timeout, long long)
    declare_setting(histogram_timeout_recording, long long)
    
    static const int DefaultServerPort;
    declare_setting(server_host, QString)
    declare_setting(server_port, int)
    
    enum NetworkImageFormat { Network_RAW=0, Network_JPEG=1, Network_NoImage=2 };
    declare_setting(server_image_format, NetworkImageFormat)
    declare_setting(server_compression, bool)
    declare_setting(server_force8bit, bool)
    declare_setting(server_jpeg_quality, int)
    
    declare_setting(last_controls_folder, QString)
    
    declare_setting(timelapse_mode, bool)
    declare_setting(timelapse_msecs, qlonglong)
    
    declare_setting(filter_presets_by_camera, bool)
    declare_setting(deprecated_video_warning_shown, bool)
    declare_setting(recording_pause_stops_timer, bool)
    
    /// Defines how to interpret multi-byte data coming from camera (CameraDefault = as reported by the camera; may be unreliable)
    enum class CaptureEndianess { CameraDefault=0, Little=1, Big=2};
    declare_setting(capture_endianess, CaptureEndianess)

    /// If true, camera control changes are applied immediately (no need to click "Apply")
    declare_setting(immediate_controls, bool);

    struct Preset {
      QString path;
      QString name;
      typedef QList<Preset> List;
      bool isValid() const;
      bool isFor(const QString &camera) const;
      QString camera() const;
      QVariantMap load() const;
      void save(const QVariantMap &presets);
    };
    
    QStringList latest_exported_presets() const;
    QStringList latest_recording_presets() const;
    
    void add_preset(const QString &name, const QVariantMap &presetValues);
    Preset::List list_presets() const;
    Preset load_preset(const QString &name) const;
    void remove_preset(const QString &name);
    
    QString savefile() const;
    
public slots:
    void preset_saved(const QString &file);
    void recording_preset_saved(const QString &file);
signals:
  void presets_changed();
  void recording_presets_changed();
  void settings_changed();
private:
    DPTR
};


#endif // CONFIGURATION_H
