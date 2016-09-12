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


#include "zwoexception.h"
#include <unordered_map>
#include "c++/stringbuilder.h"

using namespace std;
using namespace GuLinux;

ZWOException::ZWOException(int code, const string &where)
  : Imager::exception{
      code,
      unordered_map<int, string>{
        { ASI_SUCCESS, "SUCCESS: no error" },
        { ASI_ERROR_INVALID_INDEX, "INVALID_INDEX: no camera connected or index value out of boundary" },
        { ASI_ERROR_INVALID_ID, "INVALID_ID: invalid ID" },
        { ASI_ERROR_INVALID_CONTROL_TYPE, "INVALID_CONTROL_TYPE: invalid control type" },
        { ASI_ERROR_CAMERA_CLOSED, "CAMERA_CLOSED: camera didn't open" },
        { ASI_ERROR_CAMERA_REMOVED, "CAMERA_REMOVED: failed to find the camera, maybe the camera has been removed" },
        { ASI_ERROR_INVALID_PATH, "INVALID_PATH: cannot find the path of the file" },
        { ASI_ERROR_INVALID_FILEFORMAT, "INVALID_FILEFORMAT" },
        { ASI_ERROR_INVALID_SIZE, "INVALID_SIZE: wrong video format size" },
        { ASI_ERROR_INVALID_IMGTYPE, "INVALID_IMGTYPE: unsupported image formate" },
        { ASI_ERROR_OUTOF_BOUNDARY, "OUTOF_BOUNDARY: the startpos is out of boundary" },
        { ASI_ERROR_TIMEOUT, "TIMEOUT: timeout" },
        { ASI_ERROR_INVALID_SEQUENCE, "INVALID_SEQUENCE: stop capture first" },
        { ASI_ERROR_BUFFER_TOO_SMALL, "BUFFER_TOO_SMALL: buffer size is not big enough" },
        { ASI_ERROR_VIDEO_MODE_ACTIVE, "VIDEO_MODE_ACTIVE" },
        { ASI_ERROR_EXPOSURE_IN_PROGRESS, "EXPOSURE_IN_PROGRESS" },
        { ASI_ERROR_GENERAL_ERROR, "GENERAL_ERROR: general error, eg: value is out of valid range"},
    }, 
    "ASI error ",
    where
  }
{
}

