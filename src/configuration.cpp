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

class Configuration::Private {
public:
  Private(QSettings &settings, Configuration *q);
  QSettings &settings;
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
  return d->settings.value("buffered_output", true).toBool();
}

void Configuration::setBufferedOutput(bool buffered)
{
  d->settings.setValue("buffered_output", buffered);
}

long long Configuration::maxMemoryUsage() const
{
  return d->settings.value("max_save_memory_usage", 20*1024*1024).toLongLong();
}

void Configuration::setMaxMemoryUsage(long long memoryUsage)
{
  d->settings.setValue("max_save_memory_usage", memoryUsage);
}

int Configuration::maxPreviewFPSOnSaving() const
{
  return d->settings.value("max_preview_fps", 0).toInt();
}

void Configuration::setMaxPreviewFPSOnSaving(int maxFPS)
{
  d->settings.setValue("max_preview_fps", maxFPS);
}
