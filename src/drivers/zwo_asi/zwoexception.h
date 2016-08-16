/*
 * Copyright (C) 2016 Marco Gulino (marco AT gulinux.net)
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef ZWOEXCEPTION_H
#define ZWOEXCEPTION_H

#include <stdexcept>
#include "ASICamera2.h"
#include "c++/dptr.h"

class ZWOException : public std::exception {
public:
  ZWOException(ASI_ERROR_CODE code, const std::string &where = {});
  ZWOException(const ZWOException &other);
  ASI_ERROR_CODE code() const;
  virtual const char* what() const noexcept;
  class Check;
private:
  DPTR
};

class ZWOException::Check {
public:
  Check(const std::string &file, int line);
  ~Check();
  Check &operator<<(ASI_ERROR_CODE code);
  Check &operator<<(const std::string &operation);
private:
  DPTR
};
#define ASI_CHECK ZWOException::Check(__FILE__, __LINE__)

#endif // ZWOEXCEPTION_H
