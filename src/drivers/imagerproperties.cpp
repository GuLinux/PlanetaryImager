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

#include "imager.h"
#include "Qt/strings.h"

using namespace std;


QDebug operator<<(QDebug dbg, const Imager::Properties& chip)
{
  dbg.nospace() << chip.properties;
  return dbg.space();
}

QDebug operator<<(QDebug dbg, const Imager::Properties::Property& property)
{
    dbg << "{name: " << property.name << ", value: " << property.value;
    if(property.name != property.displayName())
        dbg << ", display_name: " << property.displayName();
    if(property.value != property.displayValue())
        dbg << ", display_value: " << property.displayValue();
    return dbg << ", hidden: " << property.hidden << "}";
}


Imager::Properties & Imager::Properties::operator<<(const Imager::Properties::Property& property)
{
    properties.push_back(property);
    return *this;
}


Imager::Properties & Imager::Properties::set_chip_size(double width, double height)
{
    return *this << Property{"chip_size", QVariantMap{ {"width", width}, {"height", height} },  "Chip size", "%1x%2 mm"_q % width% height };
}

Imager::Properties & Imager::Properties::set_pixel_size(double width, double height)
{
  return *this << Property{"pixel_size", QVariantMap{ {"width", width}, {"height", width} },  "Pixel size", "%1x%2 um"_q % width% width };
}

Imager::Properties & Imager::Properties::set_resolution(const QSize& resolution)
{
    return *this << Property{"resolution", resolution,  "Resolution", "%1x%2"_q % resolution.width() % resolution.height() };
}


Imager::Properties & Imager::Properties::set_pixelsize_chipsize(double pixelwidth, double pixelheight, double width, double height)
{
    QSize resolution = { static_cast<int>(width*1000./pixelwidth), static_cast<int>(height*1000./pixelheight)};
    return set_resolution(resolution).set_chip_size(width,height).set_pixel_size(pixelwidth,pixelheight);
}

Imager::Properties & Imager::Properties::set_resolution_chipsize(const QSize& resolution, double width, double height)
{
    double pixelwidth = width * 1000. / static_cast<double>(resolution.width());
    double pixelheight = height * 1000. / static_cast<double>(resolution.height());
    return set_resolution(resolution).set_chip_size(width,height).set_pixel_size(pixelwidth,pixelheight);
}

Imager::Properties & Imager::Properties::set_resolution_pixelsize(const QSize& resolution, double pixelwidth, double pixelheight)
{
    double width = pixelwidth * resolution.width() / 1000.;
    double height = pixelheight * resolution.height() / 1000.;
    return set_resolution(resolution).set_chip_size(width,height).set_pixel_size(pixelwidth,pixelheight);
}

QString Imager::Properties::Property::displayName() const
{
    return display_name.isEmpty() ? name : display_name;
}

QString Imager::Properties::Property::displayValue() const
{
    return display_value.isEmpty() ? value.toString() : display_value;
}

