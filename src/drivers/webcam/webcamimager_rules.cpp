/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Marco Gulino <marco.gulino@bhuman.it>
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


#include "webcamimager_p.h"

void WebcamImager::Private::populate_rules()
{
 // Exposure to combo in Microsoft Lifecam 3000
  setting_rules.push_back( [=](Setting &s){
   if(s.id != V4L2_CID_EXPOSURE_ABSOLUTE || (cameraname != "Microsoft\u00AE LifeCam HD-3000" && cameraname != "Microsoft\u00AE LifeCam HD-5000"))
      return;
   s.type = Setting::Combo;
   s.choices = {{"5", 5}, {"9", 9}, {"10", 10}, {"19", 19}, {"20", 20}, {"39", 39}, {"78", 78}, {"156", 156}, {"312", 312}, {"625", 625}, {"1250", 1250}, {"2500", 2500}, {"5000", 5000}, {"10000", 10000}};
  });

}
