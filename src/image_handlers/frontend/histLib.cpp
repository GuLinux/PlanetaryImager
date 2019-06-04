//=============================================================================
// Copyright (c) 2015, Paul Filitchkin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in
//      the documentation and/or other materials provided with the
//      distribution.
//
//    * Neither the name of the organization nor the names of its contributors
//      may be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//=============================================================================

#include "histLib.h"
#include <opencv2/opencv.hpp>
#include "commons/opencv_utils.h"
#include <iostream>
using namespace cv;
using namespace std;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CHistLib::CHistLib() :
  mHistImageHeight(150),
  mHistImageBorder(15),
  mBinCount(256),
  mHistPlotColor(HIST_LIB_COLOR_WHITE),
  mHistAxisColor(HIST_LIB_COLOR_WHITE),
  mHistBackgroundColor(HIST_LIB_COLOR_BLACK),
  mDrawXAxis(true),
  mSpread(1),
  mDrawSpreadOut(false)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CHistLib::~CHistLib()
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHistLib::SetHistImageHeight(unsigned HistImageHeight)
{
  if ((HistImageHeight > 0) && (HistImageHeight <= 2048))
  {
    mHistImageHeight = HistImageHeight;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHistLib::SetBinCount(unsigned BinCount)
{
  if ((BinCount > 0) && (BinCount <= 256))
  {
    mBinCount = BinCount;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHistLib::SetPlotColor(cv::Scalar Color)
{
  mHistPlotColor = Color;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHistLib::SetAxisColor(cv::Scalar Color)
{
  mHistAxisColor = Color;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHistLib::SetBackgroundColor(cv::Scalar Color)
{
  mHistBackgroundColor = Color;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHistLib::SetDrawSpreadOut(bool DrawSpreadOut)
{
  mDrawSpreadOut = DrawSpreadOut;
  if (mDrawSpreadOut)
  {
    mSpread = 3;
  }
  else
  {
    mSpread = 1;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned CHistLib::GetHistImageHeight() const
{
  return mHistImageHeight;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned CHistLib::GetBinCount() const
{
  return mBinCount;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
cv::Scalar CHistLib::GetPlotColor() const
{
  return mHistPlotColor;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
cv::Scalar CHistLib::GetAxisColor() const
{
  return mHistAxisColor;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
cv::Scalar CHistLib::GetBackgroundColor() const
{
  return mHistBackgroundColor;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHistLib::GetDrawSpreadOut() const
{
  return mDrawSpreadOut;
}

//-----------------------------------------------------------------------------
// Description:
//   General purpose histogram drawing function
//-----------------------------------------------------------------------------
void CHistLib::DrawHistogram(const Mat& Hist, Mat& HistImage)
{
  unsigned HistLength;

  // Make sure we have a row or column vector
  if (Hist.rows > Hist.cols)
  {
    if ((Hist.rows < 2) || (Hist.cols != 1))
    {
      return;
    }
    HistLength = Hist.rows;
  }
  else
  {
    if ((Hist.rows != 1) || (Hist.cols < 2))
    {
      return;
    }
    HistLength = Hist.cols;
  }

  // Should do nothing if the input is already the correct size/type
  HistImage.create(
    2 * mHistImageBorder + mHistImageHeight,
    2 * mHistImageBorder + mSpread * HistLength,
    CV_8UC3);

  HistImage.setTo(mHistBackgroundColor);

  DrawHistBins(Hist, HistImage, mHistPlotColor);

  if (mDrawXAxis)
  {
    DrawHistBar(HistImage, HistLength);
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Computes a three channel (BGR) histogram
//-----------------------------------------------------------------------------
void CHistLib::ComputeHistogramBGR(
  const cv::Mat& Image,
  cv::MatND& HistB,
  cv::MatND& HistG,
  cv::MatND& HistR)
{
  const Mat* pImageBGR;
  Mat ImageBGR;

  switch (Image.type())
  {
    case CV_8UC3:
      pImageBGR = &Image;
    break;

    case CV_8UC4:
    {
      cvtColor(Image, ImageBGR, cv::COLOR_RGBA2RGB);
      pImageBGR = &ImageBGR;
    }
    break;

    default:
      CV_Error(cv::Error::StsUnsupportedFormat, "CHistLib::DrawHistogramGray");
    break;
  }

  // Initialize histogram settings
  int histSize[] = {static_cast<int>(mBinCount)};
  float Range[] = {0, 256}; //{0, 256} = 0 to 255
  const float* Ranges[] = {Range};
  int chanB[] = {0};
  int chanG[] = {1};
  int chanR[] = {2};

  calcHist(pImageBGR, 1, chanB, Mat(), // do not use mask
    HistB,
    1,
    histSize,
    Ranges,
    true, // the histogram is uniform
    false);

  calcHist(pImageBGR, 1, chanG, Mat(), // do not use mask
    HistG,
    1,
    histSize,
    Ranges,
    true, // the histogram is uniform
    false);

  calcHist(pImageBGR, 1, chanR, Mat(), // do not use mask
    HistR,
    1,
    histSize,
    Ranges,
    true, // the histogram is uniform
    false);
}

//-----------------------------------------------------------------------------
// Description:
//   Computes a single channel (Value) histogram
//-----------------------------------------------------------------------------
void CHistLib::ComputeHistogramValue(const cv::Mat& Image, cv::MatND& Hist)
{
  // Create 1 channel image to get a value representation
  Mat ImageValue = Mat(Image.size(), CV_8UC1);

  switch (Image.type())
  {
    case CV_8UC1:
      Image.copyTo(ImageValue);
    break;

    case CV_8UC3:
    {
      Mat ImageHSV = Mat(Image.size(), CV_8UC3);
      vector<Mat> ChannlesHsv;

      cvtColor(Image, ImageHSV, cv::COLOR_BGR2HSV);
      cv::split(ImageHSV, ChannlesHsv);
      ImageValue = ChannlesHsv[2];
    }
    break;

    case CV_8UC4:
    {
      Mat ImageHSV = Mat(Image.size(), CV_8UC3);
      Mat ImageBGR = Mat(Image.size(), CV_8UC3);
      vector<Mat> ChannlesHsv;

      cvtColor(Image, ImageBGR, cv::COLOR_RGBA2RGB);
      cvtColor(ImageBGR, ImageHSV, cv::COLOR_RGBA2RGB);
      cv::split(ImageHSV, ChannlesHsv);
      ImageValue = ChannlesHsv[2];
    }
    break;

    default:
      CV_Error(cv::Error::StsUnsupportedFormat, "CHistLib::DrawHistogramValue");
    break;
  }

  // Initialize histogram settings
  int histSize[] = { static_cast<int>(mBinCount) };
  float Range[] = { 0, 256 }; //{0, 256} = 0 to 255
  const float *Ranges[] = { Range };
  int channels[] = { 0 };

  calcHist(&ImageValue, 1, channels, Mat(), // do not use mask
    Hist,
    1,
    histSize,
    Ranges,
    true, // the histogram is uniform
    false);
}

//-----------------------------------------------------------------------------
// Description:
//   Normalizes and draws a three channel (BGR) histogram
//-----------------------------------------------------------------------------
void CHistLib::DrawHistogramBGR(
  cv::MatND& HistB,
  cv::MatND& HistG,
  cv::MatND& HistR,
  cv::Mat& HistImage)
{
  double maxB = 0;
  double maxG = 0;
  double maxR = 0;

  minMaxLoc(HistB, 0, &maxB, 0, 0);
  minMaxLoc(HistG, 0, &maxG, 0, 0);
  minMaxLoc(HistR, 0, &maxR, 0, 0);

  double maxBGR = max(maxB, max(maxG, maxR));

  for (int i = 0; i < HistB.rows; ++i)
  {
    HistB.at<float>(i, 0) = (float) mHistImageHeight * HistB.at<float>(i, 0)
        / (float) maxBGR;
    HistG.at<float>(i, 0) = (float) mHistImageHeight * HistG.at<float>(i, 0)
        / (float) maxBGR;
    HistR.at<float>(i, 0) = (float) mHistImageHeight * HistR.at<float>(i, 0)
        / (float) maxBGR;
  }

  // Should do nothing if the input is already the correct size/type
  HistImage.create(
    2 * mHistImageBorder + mHistImageHeight,
    2 * mHistImageBorder + mSpread * mBinCount,
    CV_8UC3);

  HistImage.setTo(mHistBackgroundColor);

  DrawHistBins(HistB, HistImage, HIST_LIB_COLOR_BLACK);
  DrawHistBins(HistG, HistImage, HIST_LIB_COLOR_BLACK);
  DrawHistBins(HistR, HistImage, HIST_LIB_COLOR_BLACK);

  Mat AddB(HistImage.size(), HistImage.type(), HIST_LIB_COLOR_BLACK);
  Mat AddG(HistImage.size(), HistImage.type(), HIST_LIB_COLOR_BLACK);
  Mat AddR(HistImage.size(), HistImage.type(), HIST_LIB_COLOR_BLACK);

  DrawHistBins(HistB, AddB, HIST_LIB_COLOR_BLUE);
  DrawHistBins(HistG, AddG, HIST_LIB_COLOR_GREEN);
  DrawHistBins(HistR, AddR, HIST_LIB_COLOR_RED);

  add(HistImage, AddB, HistImage);
  add(HistImage, AddG, HistImage);
  add(HistImage, AddR, HistImage);

  if (mDrawXAxis)
  {
    DrawHistBar(HistImage, mBinCount);
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Normalizes and draws a single channel histogram
//-----------------------------------------------------------------------------
void CHistLib::DrawHistogramValue(cv::MatND& Hist, cv::Mat& HistImage)
{
  double maxVal = 0;
  minMaxLoc(Hist, 0, &maxVal, 0, 0);

  for (int i = 0; i < Hist.rows; ++i)
  {
    Hist.at<float>(i, 0) = (float) mHistImageHeight * Hist.at<float>(i, 0)
        / (float) maxVal;
  }

  DrawHistogram(Hist, HistImage);
}

//-----------------------------------------------------------------------------
// Description:
//   Computes and draws a single channel (value) histogram
//-----------------------------------------------------------------------------
void CHistLib::ComputeAndDrawHistogramValue(const Mat& Image, Mat& ImageHist)
{
  MatND Hist;

  ComputeHistogramValue(Image, Hist);

  DrawHistogramValue(Hist, ImageHist);
}

//-----------------------------------------------------------------------------
// Description:
//   Computes and draws a three channel (BGR) histogram
//-----------------------------------------------------------------------------
void CHistLib::ComputeAndDrawHistogramBGR(const Mat& Image, Mat& HistImage)
{
  MatND HistB;
  MatND HistG;
  MatND HistR;

  ComputeHistogramBGR(Image, HistB, HistG, HistR);

  DrawHistogramBGR(HistB, HistG, HistR, HistImage);
}

//-----------------------------------------------------------------------------
// Description:
//   Draws the x axis of the histogram plot
//-----------------------------------------------------------------------------
void CHistLib::DrawHistBar(Mat& HistImage, unsigned BinCount)
{
  // Draw the horizontal axis
  line(
    HistImage,
    Point(mHistImageBorder, mHistImageBorder + mHistImageHeight),
    Point(
      mHistImageBorder + mSpread * BinCount,
      mHistImageBorder + mHistImageHeight),
    mHistAxisColor,
    0);

  // Label initial bin
  putText(
    HistImage,
    "0",
    Point(mHistImageBorder - 3, mHistImageBorder + mHistImageHeight + 10),
    FONT_HERSHEY_SIMPLEX,
    0.3f,
    mHistAxisColor);

  // Create text to display number of histogram bins
  stringstream mBinCountSS;
  mBinCountSS << (BinCount - 1);

  // Label last bin
  putText(
    HistImage,
    mBinCountSS.str().c_str(),
    Point(
      mHistImageBorder + mSpread * BinCount - 10,
      mHistImageBorder + mHistImageHeight + 10),
    FONT_HERSHEY_SIMPLEX,
    0.3f,
    mHistAxisColor);
}

//-----------------------------------------------------------------------------
// Description:
//   Scale the value channel to the max (withouth clipping)
//-----------------------------------------------------------------------------
void CHistLib::NormalizeImageBGR(const Mat& ImageBGR, Mat& ImageBGRNorm)
{
  Mat ImageHSV     = Mat(ImageBGR.size(), CV_8UC3);
  Mat ImageHSVNorm = Mat(ImageBGR.size(), CV_8UC3);

  cvtColor(ImageBGR, ImageHSV, cv::COLOR_BGR2HSV);

  unsigned char* data     = ImageHSV.data;
  unsigned char* dataNorm = ImageHSVNorm.data;

  // The normalization procedure here is performed with efficiency in mind
  // Find min/max from the value channel
  unsigned char min = 255;
  unsigned char max = 0;
  for ( int i = 0; i < ImageHSV.rows*ImageHSV.cols; ++i)
  {
    if( data[3*i+2] > max) max = data[3*i+2];
    if( data[3*i+2] < min) min = data[3*i+2];
  }

  for ( int i = 0; i < ImageHSV.rows*ImageHSV.cols; ++i)
  {
    dataNorm[3*i]   = data[3*i];// Hue
    dataNorm[3*i+1] = data[3*i+1];//Saturation

    double newPixelDouble =
      (double)((double)data[3*i+2]-(double)min)*(255.0f/(double)(max-min));

    unsigned char newPixelByte;

    if (newPixelDouble > 255) // Take care of positive clipping
    {
      newPixelByte = 255;
    }
    else if(newPixelDouble < 0) // Take care of negative clipping
    {
      newPixelByte = 0;
    }
    else // If there is no clipping
    {
      newPixelByte = cvRound(newPixelDouble);
    }

     // Value
    dataNorm[3*i+2] = newPixelByte;

  }
  cvtColor(ImageHSVNorm, ImageBGRNorm, cv::COLOR_HSV2BGR);
}

//-----------------------------------------------------------------------------
// Description:
//   Scale the value channel to a target clipping amount
//-----------------------------------------------------------------------------
void CHistLib::NormalizeClipImageBGR(
  const Mat& ImageBGR,
  Mat& ImageBGRNorm,
  double clipPercent)
{

  Mat ImageHSV     = Mat(ImageBGR.size(), CV_8UC3);
  Mat ImageHSVNorm = Mat(ImageBGR.size(), CV_8UC3);

  cvtColor(ImageBGR, ImageHSV, cv::COLOR_BGR2HSV);

  unsigned char* data     = ImageHSV.data;
  unsigned char* dataNorm = ImageHSVNorm.data;

  unsigned bins[mBinCount];
  memset(bins, 0, mBinCount*sizeof(unsigned));
  unsigned max = mBinCount-1;
  unsigned min = 0;

  for (int i = 0; i < ImageHSV.rows*ImageHSV.cols; ++i)
  {
    bins[data[3*i+2]]++;
  }

  // Maximum number of pixels to remove from the histogram
  // This is calculated by taking a percentage of the total number of pixels
  const double clipFraction = clipPercent / 100.0f;
  unsigned pixelsToClip = cvRound(
    clipFraction * (double) (ImageHSV.rows * ImageHSV.cols));
  unsigned pixelsToClipHalf = cvRound((double) pixelsToClip / 2);
  unsigned binSum = 0;

  // Find the lower pixel bound
  if (bins[0] < pixelsToClipHalf)
  {
    binSum = bins[0];
    for (unsigned i = 1; i < 255; ++i)
    {
      binSum = binSum + bins[i];
      if (binSum > pixelsToClipHalf)
      {
        min = i;
        break;
      }
    }
  }

  // Find the upper pixel bound
  if (bins[255] < pixelsToClipHalf)
  {
    binSum = bins[255];
    for (unsigned i = 255; i > 1; i--)
    {
      binSum = binSum + bins[i];
      if (binSum > pixelsToClipHalf)
      {
        max = i;
        break;
      }
    }
  }

  for ( int i = 0; i < ImageHSV.rows*ImageHSV.cols; ++i)
  {
    dataNorm[3*i]   = data[3*i];// Hue
    dataNorm[3*i+1] = data[3*i+1];//Saturation

     // Value
    double newPixelDouble =
      (double)((double)data[3*i+2]-(double)min)*(255.0f/(double)(max-min));

    unsigned char newPixelByte;

    if (newPixelDouble > 255) // Take care of positive clipping
    {
      newPixelByte = 255;
    }
    else if(newPixelDouble < 0) // Take care of negative clipping
    {
      newPixelByte = 0;
    }
    else // If there is no clipping
    {
      newPixelByte = cvRound(newPixelDouble);
    }

    dataNorm[3*i+2] = newPixelByte;
  }
  cvtColor(ImageHSVNorm, ImageBGRNorm, cv::COLOR_HSV2BGR);
}

//-----------------------------------------------------------------------------
// Description:
//   Helper function that draws a set of bins in the desired color
//-----------------------------------------------------------------------------
void CHistLib::DrawHistBins(
  const Mat& Hist,
  Mat& HistImage,
  const cv::Scalar& Color)
{
  // Draw the bins
  unsigned binValue = 0;

  switch (Hist.type())
  {
    // When Hist contains floats
    case CV_32F:
      if (Hist.cols > Hist.rows)
      {
        for (int i = 0; i < Hist.cols; ++i)
        {
          DrawHistBin(HistImage, Hist.at<float>(0, i), i * mSpread, Color);
        }
      }
      else
      {
        for (int i = 0; i < Hist.rows; ++i)
        {
          DrawHistBin(HistImage, Hist.at<float>(i, 0), i * mSpread, Color);
        }
      }
    break;

    // When Hist contains doubles
    case CV_64F:
      if (Hist.cols > Hist.rows)
      {
        for (int i = 0; i < Hist.cols; ++i)
        {
          DrawHistBin(HistImage, Hist.at<double>(0, i), i * mSpread, Color);
        }
      }
      else
      {
        for (int i = 0; i < Hist.rows; ++i)
        {
          DrawHistBin(HistImage, Hist.at<double>(i, 0), i * mSpread, Color);
        }
      }
    break;

    // When Hist contains ints
    case CV_32S:
      if (Hist.cols > Hist.rows)
      {
        for (int i = 0; i < Hist.cols; ++i)
        {
          DrawHistBin(HistImage, Hist.at<int>(0, i), i * mSpread, Color);
        }
      }
      else
      {
        for (int i = 0; i < Hist.rows; ++i)
        {
          DrawHistBin(HistImage, Hist.at<int>(i, 0), i * mSpread, Color);
        }
      }
    break;

    default:
      return;
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Helper function that draws a bin on the histogram image
//-----------------------------------------------------------------------------
void CHistLib::DrawHistBin(
  Mat& HistLayer,
  int Value,
  unsigned x,
  const cv::Scalar& Color)
{
  unsigned PixelHeight = (unsigned)Value;
  if (PixelHeight)
  {
    const unsigned BaseX = x + mHistImageBorder;
    const unsigned BaseY = mHistImageBorder + mHistImageHeight;

    line(
      HistLayer,
      Point(BaseX, BaseY),
      Point(BaseX, BaseY - PixelHeight),
      Color);
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Helper function that draws a bin on the histogram image.
//   Overloaded to accept a float for the bin height
//-----------------------------------------------------------------------------
void CHistLib::DrawHistBin(
  Mat& HistLayer,
  float Value,
  unsigned x,
  const cv::Scalar& Color)
{
  unsigned PixelHeight = (unsigned)cvRound(Value);
  if (PixelHeight)
  {
    const unsigned BaseX = x + mHistImageBorder;
    const unsigned BaseY = mHistImageBorder + mHistImageHeight;

    line(
      HistLayer,
      Point(BaseX, BaseY),
      Point(BaseX, BaseY - PixelHeight),
      Color);
  }
}

//-----------------------------------------------------------------------------
// Description:
//   Helper function that draws a bin on the histogram image.
//   Overloaded to accept a double for the bin height
//-----------------------------------------------------------------------------
void CHistLib::DrawHistBin(
  Mat& HistLayer,
  double Value,
  unsigned x,
  const cv::Scalar& Color)
{
  unsigned PixelHeight = (unsigned)cvRound(Value);
  if (PixelHeight)
  {
    const unsigned BaseX = x + mHistImageBorder;
    const unsigned BaseY = mHistImageBorder + mHistImageHeight;

    line(
      HistLayer,
      Point(BaseX, BaseY),
      Point(BaseX, BaseY - PixelHeight),
      Color);
  }
}
