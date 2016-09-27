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

#include "serimager.h"
using namespace std;

DPTR_IMPL(SERImager) {
};

SERImager::SERImager(const ImageHandler::ptr& handler)
{
}

SERImager::~SERImager()
{
}

Imager::Properties SERImager::properties() const
{
  return {};
}

void SERImager::clearROI()
{
}

void SERImager::setROI(const QRect&)
{
}

Imager::Controls SERImager::controls() const
{
  return {};
}

QString SERImager::name() const
{
  return "SER Player";
}

void SERImager::setControl(const Imager::Control& setting)
{
}

void SERImager::startLive()
{
}

void SERImager::stopLive()
{
}



