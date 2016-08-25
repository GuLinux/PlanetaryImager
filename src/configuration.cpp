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

#include "configuration.h"
#include <QSettings>
#include "utils.h"
#include <QMetaType>
#include "Qt/strings.h"
#include <QDir>
#include <QDateTime>
#include <QProcessEnvironment>
#include <QCoreApplication>

using namespace std;
DPTR_IMPL(Configuration) {
  shared_ptr<QSettings> settings;
  Configuration *q;
  template<typename T> T value(const QString &key, const T &defaultValue = {}) const;
  template<typename T> void set(const QString &key, const T &value);
  mutable QHash<QString, QVariant> values_cache;
};

Configuration::Configuration() : dptr(make_shared<QSettings>("GuLinux", qApp->applicationName()), this)
{
}

Configuration::~Configuration()
{
}

std::shared_ptr<QSettings> Configuration::qSettings() const
{
  return d->settings;
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
define_setting(max_memory_usage, long long, 20*1024*1024)
define_setting(buffered_output, bool, true)
define_setting(max_display_fps, int, 30)
define_setting(max_display_fps_recording, int, 5)
define_setting(limit_fps, bool, true)
define_setting(limit_fps_recording, bool, true)

define_setting(recording_frames_limit, long long, 0)
define_setting(save_file_prefix, QString, {})
define_setting(save_file_suffix, QString, {})
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
define_setting(canny_kernel_size, int, 3)
define_setting(canny_blur_size, int, 3)

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
define_setting(save_info_file, bool, true)
define_setting(widgets_setup_first_run, bool, false)
define_setting(histogram_bins, int, 50)
define_setting(histogram_enabled, bool, true)
define_setting(histogram_disable_on_recording, bool, true)
define_setting(histogram_timeout, long long, 3'500)
define_setting(histogram_timeout_recording, long long, 10'000)


QString Configuration::savefile() const
{
  QMap<SaveFormat, QString> extension {
    { SER, ".ser" },
    { Video, ".mkv" },
  };
  return "%1%2%3%4%5%6"_q
    % save_directory()
    % QDir::separator()
    % save_file_prefix()
    % QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz_t")
    % save_file_suffix()
    % extension[save_format()]
    ;
}
