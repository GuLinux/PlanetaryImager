/*
 * GuLinux Planetary Imager - https://github.com/GuLinux/PlanetaryImager
 * Copyright (C) 2017  Marco Gulino <marco@gulinux.net>
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
 */

#include "gtest/gtest.h"
#include <opencv2/opencv.hpp>
#include "commons/frame.h"

using namespace std;

cv::Mat testMat() {
  cv::Mat testImage{cv::Size{3, 2}, CV_8UC3};
  testImage.at<cv::Vec3b>(cv::Point(0, 0)) = cv::Vec3b(0, 1, 2);
  testImage.at<cv::Vec3b>(cv::Point(0, 1)) = cv::Vec3b(255, 0, 0);
  testImage.at<cv::Vec3b>(cv::Point(1, 0)) = cv::Vec3b(0, 255, 0);
  testImage.at<cv::Vec3b>(cv::Point(1, 1)) = cv::Vec3b(0, 0, 255);
  testImage.at<cv::Vec3b>(cv::Point(2, 0)) = cv::Vec3b(125, 125, 125);
  testImage.at<cv::Vec3b>(cv::Point(2, 1)) = cv::Vec3b(255, 255, 255);
  return testImage;
}


TEST(TestFrame, testFrameMatConstructor)
{
  auto testImage = testMat();
  auto frame = make_shared<Frame>(Frame::ColorFormat::BGR, testImage, Frame::LittleEndian);
  ASSERT_EQ(3, frame->channels());
  ASSERT_EQ(testImage.elemSize() * testImage.total(), frame->size());
  for(int x = 0; x < testImage.cols; x++)
    for(int y=0; y < testImage.rows; y++)
      ASSERT_EQ(testImage.at<cv::Vec3b>(cv::Point(x, y)), frame->mat().at<cv::Vec3b>(cv::Point(x, y)));
}
