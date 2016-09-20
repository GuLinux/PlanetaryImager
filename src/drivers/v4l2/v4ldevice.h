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

#ifndef V4LDEVICE_H
#define V4LDEVICE_H
#include <QString>

class V4L2Device {
public:
  V4L2Device(const QString &path);
  ~V4L2Device();
  inline QString path() const { return _path; }
  inline operator bool() const { return fd != -1; }
  int descriptor() const { return fd; }
  template<typename T> void ioctl(uint64_t ctl, T *data, const QString &errorLabel = {}) const { return __ioctl(ctl, reinterpret_cast<void*>(data), errorLabel); }
  template<typename T> int xioctl(uint64_t ctl, T *data, const QString &errorLabel = {}) const { return __xioctl(ctl, reinterpret_cast<void*>(data), errorLabel); }
  class exception : public std::exception {
  public:
     exception(const QString &label = {}) : label{label}, _error_code{errno} {}
     virtual const char* what() const noexcept;
     int error_code() const { return _error_code; }
  private:
    const QString label;
    int _error_code;
  };
private:
  int fd = -1;
  const QString _path;
  void __ioctl(uint64_t ctl, void *data, const QString &errorLabel) const;
  int __xioctl(uint64_t ctl, void *data, const QString &errorLabel) const;
};




#endif // V4LDEVICE_H
