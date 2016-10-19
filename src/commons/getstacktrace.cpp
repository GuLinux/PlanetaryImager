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
#include "getstacktrace.h"
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <mutex>
using namespace std;
StackFrame::vector StackFrame::trace(uint32_t max_frames, uint32_t skip)
{
  static mutex _mutex;
  unique_lock<mutex> lock(mutex);
  skip++; // we have to skip current frame, at least
  std::vector<void*> addresses(max_frames + skip);
  int num_addresses = backtrace(addresses.data(), addresses.size());

  if(num_addresses == 0)
    return {};
  vector frames(num_addresses);
  char** symbols = backtrace_symbols(addresses.data(), addresses.size());
  int current_frame = 0;
  for(auto &frame: frames) {
    frame.address = addresses[current_frame];
    frame.symbol = string{symbols[current_frame++]};
  }
  free(symbols);
  vector filtered_frames(frames.size() - skip);
  move(frames.begin() + skip, frames.end(), filtered_frames.begin());
  return filtered_frames;
}

std::string StackFrame::file() const
{
  size_t parenthesis = symbol.find('(');
  if(parenthesis == string::npos)
    return {};
  return symbol.substr(0, parenthesis-1);
}

std::string StackFrame::function() const
{
  size_t opening_parenthesis = symbol.find('(');
  size_t closing_parenthesis = symbol.find(')');
  if(opening_parenthesis == string::npos || closing_parenthesis == string::npos)
    return{};
  string function_mangled = symbol.substr(opening_parenthesis+1, closing_parenthesis-opening_parenthesis-1);
  size_t plus_index = function_mangled.find('+');
  string offset;
  if( plus_index != string::npos) {
    offset = function_mangled.substr(plus_index, function_mangled.size()-plus_index-1);
    function_mangled = function_mangled.substr(0, plus_index);
  }
  int status;
  char *demangled = abi::__cxa_demangle(function_mangled.c_str(), 0, 0, &status);
  if(status == 0) {
    string function_demangled{demangled};
    auto full_function = function_demangled + offset;
    free(demangled);
    return full_function;
  }
  return function_mangled;
  
}


ostream &operator<<(ostream& o, const StackFrame& frame)
{
  o << "address: " << frame.address << ", file: " << frame.file() << ", function: " << frame.function() ; //<< ", symbol: [[" << frame.symbol << "]]";
  return o;
}

ostream &operator<<(ostream& o, const StackFrame::vector& frames)
{
  int index = 0;
  for(auto &frame: frames) {
    o << "[" << index++ << "]: " << frame << endl;
  }
  return o;
}
