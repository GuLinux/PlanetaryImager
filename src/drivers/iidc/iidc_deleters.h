/*
 * Copyright (C) 2017 Filip Szczerek <ga.software@yahoo.com>
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

#ifndef IIDC_DELETERS_H
#define IIDC_DELETERS_H

#include <dc1394/dc1394.h>


/// Deleters of libdc1394 objects
namespace Deleters
{
    struct dc1394      { void operator ()(dc1394_t *ptr)            const { dc1394_free(ptr); } };
    struct camera_list { void operator ()(dc1394camera_list_t *ptr) const { dc1394_camera_free_list(ptr); } };
    struct camera      { void operator ()(dc1394camera_t *ptr)      const { dc1394_camera_free(ptr); } };
}

#endif // IIDC_DELETERS_H
