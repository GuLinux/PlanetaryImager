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

class Configuration::Private {
public:
  Private(QSettings &settings, Configuration *q);
  QSettings &settings;
  template<typename T> T value(const QString &key, const T &defaultValue = {}) const;
  template<typename T> void set(const QString &key, const T &value);
  mutable QVariantMap values_cache;
private:
  Configuration *q;
};

Configuration::Private::Private(QSettings& settings, Configuration* q) : settings(settings), q(q)
{
}


Configuration::Configuration(QSettings &settings) : dptr(settings, this)
{
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
    found_value = settings.value(key, defaultValue);
    values_cache[key] = found_value;
  }
  return qvariant_cast<T>(found_value);
}


template<typename T> void Configuration::Private::set(const QString& key, const T& value)
{
  settings.setValue(key, value);
  values_cache[key] = value;
}


bool Configuration::bufferedOutput() const
{
  return d->value("buffered_output", true);
}

void Configuration::setBufferedOutput(bool buffered)
{
  d->set("buffered_output", buffered);
}

long long Configuration::maxMemoryUsage() const
{
  return d->value("max_save_memory_usage", 20*1024*1024);
}

void Configuration::setMaxMemoryUsage(long long memoryUsage)
{
  d->set("max_save_memory_usage", memoryUsage);
}

int Configuration::maxPreviewFPSOnSaving() const
{
  return d->value("max_preview_fps", 0);
}

void Configuration::setMaxPreviewFPSOnSaving(int maxFPS)
{
  d->set("max_preview_fps", maxFPS);
}


long long int Configuration::recordingFramesLimit() const
{
  return d->value("recording_frames_limit", 0);
}

void Configuration::setRecordingFramesLimit(long long int limit)
{
  d->set("recording_frames_limit", limit);
}

QString Configuration::saveFilePrefix() const
{
  return d->value<QString>("save_file_prefix");
}

void Configuration::setSaveFilePrefix(const QString& prefix)
{
  d->set("save_file_prefix", prefix);
}


QString Configuration::saveFileSuffix() const
{
  return d->value<QString>("save_file_suffix");
}

void Configuration::setSaveFileSuffix(const QString& suffix)
{
  d->set("save_file_suffix", suffix);
}

QString Configuration::saveDirectory() const
{
  return d->value<QString>("save_directory");
}

void Configuration::setSaveDirectory(const QString& directory)
{
  d->set("save_directory", directory);
}

Configuration::SaveFormat Configuration::saveFormat() const
{
  return static_cast<SaveFormat>(d->value<int>("save_format", SER));
}

void Configuration::setSaveFormat(Configuration::SaveFormat format)
{
  d->set("save_format", static_cast<int>(format));
}

QString Configuration::observer() const
{
  return d->value<QString>("save_observer");
}

void Configuration::setObserver(const QString& observer)
{
  d->set<QString>("save_observer", observer);
}

QString Configuration::telescope() const
{
  return d->value<QString>("save_telescope");
}

void Configuration::setTelescope(const QString& telescope)
{
  d->set<QString>("save_telescope", telescope);
}

Configuration::EdgeAlgorithm Configuration::edgeAlgorithm() const
{
  return static_cast<EdgeAlgorithm>(d->value<int>("edge_algorithm", Canny));
}

void Configuration::setEdgeAlgorithm(Configuration::EdgeAlgorithm algorithm)
{
  d->set("edge_algorithm", static_cast<int>(algorithm));
}

int Configuration::sobelKernel() const
{
  return d->value<int>("sobel_kernel_size", 3);
}

void Configuration::setSobelKernel(int size)
{
  d->set("sobel_kernel_size", size);
}

int Configuration::sobelBlurSize() const
{
    return d->value<int>("sobel_blur_size", 3);
}

void Configuration::setSobelBlurSize(int size)
{
    d->set("sobel_blur_size", size);
}


double Configuration::cannyLowThreshold() const
{
  return d->value<double>("canny_low_threshold", 1);
}

void Configuration::setCannyLowThreshold(double threshold)
{
  d->set<double>("canny_low_threshold", threshold);
}


double Configuration::cannyThresholdRatio() const
{
  return d->value<double>("canny_threshold_ratio", 3);
}

void Configuration::setCannyThresholdRatio(double ratio)
{
  d->set<double>("canny_threshold_ratio", ratio);
}

void Configuration::setSobelDelta(double delta)
{
  d->set<double>("sobel_delta", delta);
}

double Configuration::sobelDelta() const
{
  return d->value<double>("sobel_delta", 0);
}

double Configuration::sobelScale() const
{
  return d->value<double>("sobel_scale", 1);
}

void Configuration::setSobelScale(double scale)
{
  d->set<double>("sobel_scale", scale);
}

int Configuration::cannyKernelSize() const
{
  return d->value("canny_kernel", 3);
}

void Configuration::setCannyKernelSize(int size)
{
  d->set("canny_kernel", size);
}

int Configuration::cannyBlurSize() const
{
  return d->value("canny_blur", 3);
}

void Configuration::setCannyBlurSize(int size)
{
  d->set("canny_blur", size);
}

void Configuration::resetCannyAdvancedSettings()
{
    d->settings.remove("canny_blur");
    d->settings.remove("canny_kernel");
    d->settings.remove("canny_threshold_ratio");
    d->settings.remove("canny_low_threshold");
    d->values_cache.clear();
}

void Configuration::resetSobelAdvancedSettings()
{
    d->settings.remove("sobel_blur_size");
    d->settings.remove("sobel_delta");
    d->settings.remove("sobel_scale");
    d->settings.remove("sobel_kernel_size");
    d->values_cache.clear();
}

void Configuration::setVideoCodec(const QString &codec)
{
    d->set<QString>("video_codec", codec);
}

QString Configuration::videoCodec() const
{
    return d->value<QString>("video_codec", "HFYU");
}


bool Configuration::save_info_file() const
{
  return d->value<bool>("save_info_file", true);
}

void Configuration::set_save_info_file(bool save)
{
  d->set<bool>("save_info_file",save);
}



QString Configuration::savefile() const
{
  QMap<SaveFormat, QString> extension {
    { SER, ".ser" },
    { Video, ".mkv" },
  };
  return "%1%2%3%4%5%6"_q
    % saveDirectory()
    % QDir::separator()
    % saveFilePrefix()
    % QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz_t")
    % saveFileSuffix()
    % extension[saveFormat()]
    ;
}

void Configuration::set_widgets_setup_first_run()
{
  d->set<bool>("widgets_setup_first_run",true);
}

bool Configuration::widgets_setup_first_run() const
{
  return d->value<bool>("widgets_setup_first_run", false);
}

int Configuration::histogram_bins() const
{
  return d->value<int>("histogram-bins", 50);
}

void Configuration::set_histogram_bins(int bins)
{
  d->set<int>("histogram-bins", bins);
}

