/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

Q_DECLARE_METATYPE(Configuration::SaveFormat);


class Configuration::Private {
public:
  Private(QSettings &settings, Configuration *q);
  QSettings &settings;
  template<typename T> T value(const QString &key, const T &defaultValue = {}) const { return qvariant_cast<T>(settings.value(key, defaultValue)); }
  template<typename T> void set(const QString &key, const T &value) { settings.setValue(key, value); }
private:
  Configuration *q;
};

Configuration::Private::Private(QSettings& settings, Configuration* q) : settings(settings), q(q)
{
}


Configuration::Configuration(QSettings &settings) : dpointer(settings, this)
{
}

Configuration::~Configuration()
{
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
  return d->value<SaveFormat>("save_format", SER);
}

void Configuration::setSaveFormat(Configuration::SaveFormat format)
{
  d->set("save_format", format);
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


QString Configuration::savefile() const
{
  QMap<SaveFormat, QString> extension {
    { SER, ".ser" },
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
