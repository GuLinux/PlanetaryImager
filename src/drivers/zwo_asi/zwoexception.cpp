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
#include <map>
#include "c++/stringbuilder.h"

using namespace std;
using namespace GuLinux;

DPTR_IMPL(ZWOException) {
  const ASI_ERROR_CODE code;
  string what;
};

ZWOException::ZWOException(ASI_ERROR_CODE code, const string &where)
  : dptr(code)
{
  static map<ASI_ERROR_CODE, string> zwo_errors = {
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
  };
  ostringstream message;
  message << "ASI Error " << zwo_errors[code] << " (code: " << code << ")";
  if(!where.empty())
    message << " on " << where;
  d->what = message.str();
}

ZWOException::ZWOException(const ZWOException &other) : dptr(other.d->code, other.d->what)
{
}

ASI_ERROR_CODE ZWOException::code() const
{
  return d->code;
}

const char *ZWOException::what() const noexcept
{
  return d->what.c_str();
}

DPTR_IMPL(ZWOException::Check) {
  const std::string file;
  const int line;
  string operation;
  ASI_ERROR_CODE code = ASI_SUCCESS;
};

ZWOException::Check::Check(const string &file, int line) : dptr(file, line)
{
}

ZWOException::Check::~Check()
{
  if(d->code != ASI_SUCCESS) {
    string where = d->operation.empty()
        ? (stringbuilder() << d->file << ":" << d->line)
        : (stringbuilder() << d->operation << " (" << d->file << ":" << d->line << ")");
    throw ZWOException{d->code, where};
  }
}

ZWOException::Check &ZWOException::Check::operator<<(ASI_ERROR_CODE code)
{
  d->code = code;
  return *this;
}

ZWOException::Check &ZWOException::Check::operator<<(const string &operation)
{
  d->operation = operation;
  return *this;
}

