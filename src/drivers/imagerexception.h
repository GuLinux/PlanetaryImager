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

#ifndef GULINUX_PLANETARTYIMAGER_EXCEPTION_H
#define GULINUX_PLANETARTYIMAGER_EXCEPTION_H

#include "imager.h"
#include <stdexcept>
#include <unordered_map>
#include "c++/dptr.h"

class Imager::exception : public std::exception {
public:
    int code() const;
    virtual const char* what() const noexcept;
    virtual ~exception();
    exception(const exception &other);
    template<typename A, typename B> class check;
protected:
     exception(int code, const std::unordered_map<int, std::string> codes_map, const std::string &message_prefix = {}, const std::string &where = {});
private:
    DPTR
};

#include "c++/stringbuilder.h"
template<typename FailCondition, typename ExceptionClass>
class Imager::exception::check {
public:
  check(const std::string &file, int line, int ok_code = 0) : file{file}, line{line}, code{ok_code}, ok_code{ok_code}, fail_condition{} {}
  ~check() {
        if( fail_condition(code, ok_code)) {
            std::string where = operation.empty()
                ? (GuLinux::stringbuilder() << file << ":" << line)
                : (GuLinux::stringbuilder() << operation << " (" << file << ":" << line << ")");
        throw ExceptionClass(code, where);
    }
  }
  check &operator<<(int code) { this->code = code; return *this; }
  check &operator<<(const std::string &operation) { this->operation = operation; return *this; }
private:
  const std::string file;
  const int line;
  const FailCondition fail_condition;
  std::string operation;
  int code, ok_code;
};
#define C_ERROR_CHECK(check_type, exception_class, ok_code) Imager::exception::check<check_type, exception_class>(__FILE__, __LINE__, ok_code)

#endif
