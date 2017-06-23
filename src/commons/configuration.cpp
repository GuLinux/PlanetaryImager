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

#include "commons/configuration.h"
#include <QSettings>
#include "commons/utils.h"
#include <QMetaType>
#include "Qt/strings.h"
#include <QDir>
#include <QDateTime>
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QFileInfo>
#include <QJsonDocument>

using namespace std;
using namespace std::placeholders;
DPTR_IMPL(Configuration) {
  shared_ptr<QSettings> settings;
  Configuration *q;
  template<typename T> T value(const QString &key, const T &defaultValue = {}) const;
  template<typename T> void set(const QString &key, const T &value);
  mutable QHash<QString, QVariant> values_cache;
  QDir profilesPath;
  QString presetPath(const QString &name) const;
  
  QStringList latest_presets(const QString &configName);
  void preset_added(const QString &path, const QString &configName);
};

const int Configuration::DefaultServerPort = 19232;

Configuration::Configuration() : dptr(make_shared<QSettings>("GuLinux", qApp->applicationName()), this)
{
  QDir appDataPath{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)};
  d->profilesPath.setPath(appDataPath.path() + "/profiles");
  if(! d->profilesPath.exists())
    d->profilesPath.mkpath(".");
}

Configuration::~Configuration()
{
}

template<typename T> T Configuration::Private::value(const QString& key, const T& defaultValue) const
{
  QVariant found_value;
  if(values_cache.count(key)) {
    found_value = values_cache[key];
  } else {
    found_value = settings->value(key, defaultValue);
    values_cache[key] = found_value;
  }
  return qvariant_cast<T>(found_value);
}


template<typename T> void Configuration::Private::set(const QString& key, const T& value)
{
  settings->setValue(key, value);
  values_cache[key] = value;
}

#define define_setting_reset(name) void Configuration::reset_ ##name() { d->settings->remove(#name); d->values_cache.remove(#name); }
#define define_setting_set(name, type) void Configuration::set_ ##name(const type &value) { d->set<type>( #name, value); }
#define define_setting_enum_set(name, type) void Configuration::set_ ##name(const type &value) { d->set<int>( #name, static_cast<int>(value) ); }
#define define_setting_get(name, type, default_value) type Configuration::name() const { return d->value<type>(#name, default_value); }
#define define_setting_enum_get(name, type, default_value) type Configuration::name() const { return static_cast<type>(d->value<int>(#name, static_cast<int>(default_value) )); }

#define define_setting(name, type, default_value) define_setting_get(name, type, default_value)  define_setting_set(name, type) define_setting_reset(name)
#define define_setting_enum(name, type, default_value) define_setting_enum_get(name, type, default_value)  define_setting_enum_set(name, type) define_setting_reset(name)

define_setting(dock_status, QByteArray, {})
define_setting(main_window_geometry, QByteArray, {})
define_setting(max_memory_usage, long long, 1024*1024*1024)
define_setting(buffered_output, bool, true)
define_setting(max_display_fps, int, 30)
define_setting(max_display_fps_recording, int, 15)
define_setting(limit_fps, bool, true)
define_setting(debayer, bool, true)
define_setting(opengl, bool, true)
define_setting(limit_fps_recording, bool, true)

define_setting_enum(recording_limit_type, Configuration::RecordingLimit, Configuration::FramesNumber)
define_setting(recording_seconds_limit, double , 5)    
define_setting(recording_frames_limit, long long, 100)

define_setting(save_file_prefix, QString, {})
define_setting(save_file_suffix, QString, {})
define_setting(save_file_prefix_suffix_separator, QString, "-")
define_setting(save_file_avail_prefixes, QStringList, (QStringList{"Sun", "Moon", "Jupiter", "Saturn", "Venus", "Mercury", "Mars", "Uranus", "Neptune"}))
define_setting(save_file_avail_suffixes, QStringList, (QStringList{"L", "R", "G", "B", "IR", "Flat", "Dark", "Light", "Bias"}))

define_setting(save_directory, QString, QProcessEnvironment::systemEnvironment().value("HOME"))
define_setting(telescope, QString, {})
define_setting(observer, QString, {})
define_setting_enum(edge_algorithm, Configuration::EdgeAlgorithm, Configuration::Canny)
define_setting(sobel_kernel, int, 3)
define_setting(sobel_blur_size, int, 3)
define_setting(canny_low_threshold, double, 1)
define_setting(canny_threshold_ratio, double, 3)
define_setting(sobel_delta, double, 0)
define_setting(sobel_scale, double, 1)
define_setting(canny_kernel_size, int, 7)
define_setting(canny_blur_size, int, 5)

void Configuration::resetCannyAdvancedSettings()
{
  reset_canny_blur_size();
  reset_canny_kernel_size();
  reset_canny_low_threshold();
  reset_canny_threshold_ratio();
}

void Configuration::resetSobelAdvancedSettings()
{
  reset_sobel_blur_size();
  reset_sobel_delta();
  reset_sobel_kernel();
  reset_sobel_scale();
}

define_setting_enum(save_format, Configuration::SaveFormat, Configuration::SER)
define_setting(video_codec, QString, "HFYU")
define_setting(save_json_info_file, bool, true)
define_setting(save_info_file, bool, true)
define_setting(widgets_setup_first_run, bool, false)
define_setting(histogram_bins, int, 255)
define_setting(histogram_logarithmic, bool, true)
define_setting(histogram_disable_on_recording, bool, false)
define_setting(histogram_timeout, long long, 2'000)
define_setting(histogram_timeout_recording, long long, 4'000)
define_setting(last_controls_folder, QString, QString(qgetenv("HOME")))

define_setting(server_host, QString, "localhost")
define_setting(server_port, int, Configuration::DefaultServerPort)
define_setting_enum(server_image_format, Configuration::NetworkImageFormat, Configuration::Network_RAW)
define_setting(server_compression, bool, false)
define_setting(server_force8bit, bool, true)
define_setting(server_jpeg_quality, int, 85)

define_setting(timelapse_mode, bool, false)
define_setting(timelapse_msecs, qlonglong, 1000)

define_setting(filter_presets_by_camera, bool, true)
define_setting(deprecated_video_warning_shown, bool, false)
define_setting(recording_pause_stops_timer, bool, false)

define_setting_enum(capture_endianess, Configuration::CaptureEndianess, Configuration::CaptureEndianess::CameraDefault)

QString Configuration::savefile() const
{
  QMap<SaveFormat, QString> extension {
    { SER, ".ser" },
    { Video, ".mkv" },
  };
  return "%1%2%3%4%5%6"_q
    % save_directory()
    % QDir::separator()
    % (save_file_prefix().isEmpty() ? QString{} : "%1%2"_q % save_file_prefix() % save_file_prefix_suffix_separator())
    % QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_t")
    % (save_file_suffix().isEmpty() ? QString{} : "%1%2"_q % save_file_prefix_suffix_separator() % save_file_suffix())
    % extension[save_format()]
    ;
}

void Configuration::preset_saved(const QString& file)
{
  d->preset_added(file, "latest_preset_files");
  emit presets_changed();
}

void Configuration::recording_preset_saved(const QString& file)
{
  d->preset_added(file, "latest_recording_files");
  emit recording_presets_changed();
}

void Configuration::Private::preset_added(const QString& path, const QString& configName)
{
  QStringList saved = settings->value(configName).toStringList().mid(0, 20);
  saved.removeAll(path);
  saved.push_front(path);
  settings->setValue(configName, saved);
}


QStringList Configuration::latest_exported_presets() const
{
  return d->latest_presets("latest_preset_files");
}

QStringList Configuration::latest_recording_presets() const
{
  return d->latest_presets("latest_recording_files");
}

QStringList Configuration::Private::latest_presets(const QString& configName)
{
  auto controlFiles = settings->value(configName).toStringList();
  controlFiles.erase(remove_if(begin(controlFiles), end(controlFiles), [](const QString &f) { return !QFile::exists(f); }), controlFiles.end());
  return controlFiles.mid(0, 10);
}


void Configuration::add_preset(const QString& name, const QVariantMap& presetValues)
{
  remove_preset(name);
  Preset preset = load_preset(name);
  preset.save(presetValues);
  auto presets = list_presets();
  presets.push_front(preset);
  QStringList presetNames;
  transform(presets.begin(), presets.end(), back_inserter(presetNames), [](const auto &p) { return p.name; });
  d->settings->setValue("presets", presetNames);
}

Configuration::Preset::List Configuration::list_presets() const
{
  auto presetNames = d->settings->value("presets").toStringList();
  Preset::List presets;
  transform(begin(presetNames), end(presetNames), back_inserter(presets), bind(&Configuration::load_preset, this, _1));
  presets.erase(remove_if(begin(presets), end(presets), [](const auto p){ return !p.isValid();}), end(presets));
  return presets;
}

Configuration::Preset Configuration::load_preset(const QString &name) const
{
  return Preset{d->presetPath(name), name};
}

void Configuration::remove_preset(const QString& name)
{
  auto names = d->settings->value("presets").toStringList();
  names.removeAll(name);
  d->settings->setValue("presets", names);
  QFile::remove(d->presetPath(name));
}

QString Configuration::Private::presetPath(const QString& name) const
{
  return profilesPath.filePath(name + ".json");
}

QString Configuration::Preset::camera() const
{
  return load()["camera"].toString();
}

bool Configuration::Preset::isFor(const QString& camera) const
{
  return this->camera() == camera;
}

bool Configuration::Preset::isValid() const
{
  return QFile::exists(path);
}

QVariantMap Configuration::Preset::load() const
{
  QFile file{path};
  if(! file.open(QFile::ReadOnly)) {
    throw runtime_error(("Unable to open file %1 for reading"_q % path).toStdString());
  }
  return QJsonDocument::fromJson(file.readAll()).toVariant().toMap();
}

void Configuration::Preset::save(const QVariantMap& presets)
{
  QFile file{path};
  if(! file.open(QFile::WriteOnly)) {
    throw runtime_error(("Unable to open file %1 for writing"_q % path).toStdString());
  }
  file.write(QJsonDocument::fromVariant(presets).toJson());
}

 


#include "configuration.moc"
