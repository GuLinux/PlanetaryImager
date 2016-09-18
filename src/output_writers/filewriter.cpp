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
#include "filewriter.h"

#include "serwriter.h"
#ifdef HAVE_CV_VIDEO
#include "cvvideowriter.h"
#endif

using namespace std;
QMap< Configuration::SaveFormat, FileWriter::Factory > FileWriter::factories()
{
  return {
    {Configuration::SER, [](const QString &deviceName, Configuration &configuration){ return make_shared<SERWriter>(deviceName, configuration); }},
    {Configuration::Video, [](const QString &deviceName, Configuration &configuration){ return make_shared<cvVideoWriter>(deviceName, configuration); }},
  };
}
