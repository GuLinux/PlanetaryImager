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

#ifndef ROI_H
#define ROI_H
#include "c++/dptr.h"
#include <QRect>
#include <functional>
#include <list>

class ROIValidator {
public:
  typedef std::function<void(QRect &)> Rule;
  typedef std::shared_ptr<ROIValidator> ptr;
  ROIValidator(const std::list<Rule> &rules);
  ROIValidator(const std::initializer_list<Rule> &rules);
  ~ROIValidator();
  QRect validate(const QRect & original) const;
  static Rule max_resolution(const QRect &max);
  static Rule width_multiple(int factor);
  static Rule height_multiple(int factor);
  static Rule x_multiple(int factor);
  static Rule y_multiple(int factor);
  static Rule area_multiple(int factor, int width_step, int height_step, const QRect &fallback = QRect{});
private:
  DPTR
};

#endif // ROI_H
