//=============================================================================
// Copyright (c) 2012, Paul Filitchkin
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

#ifndef HIST_LIB
#define HIST_LIB

#define HIST_LIB_COLOR_WHITE  cv::Scalar(0xff, 0xff, 0xff)
#define HIST_LIB_COLOR_BLACK  cv::Scalar(0x00, 0x00, 0x00)
#define HIST_LIB_COLOR_BLUE   cv::Scalar(0xff, 0x00, 0x00)
#define HIST_LIB_COLOR_GREEN  cv::Scalar(0x00, 0xff, 0x00)
#define HIST_LIB_COLOR_RED    cv::Scalar(0x00, 0x00, 0xff)

#include <opencv2/core/core.hpp>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class CHistLib
{
  public:
    CHistLib();
    ~CHistLib();

    //---------
    // Setters
    //---------
    void SetHistImageHeight(unsigned HistImageHeight);
    void SetBinCount(unsigned BinCount);
    void SetPlotColor(cv::Scalar Color);
    void SetAxisColor(cv::Scalar Color);
    void SetBackgroundColor(cv::Scalar Color);
    void SetDrawSpreadOut(bool DrawSpreadOut);

    //---------
    // Getters
    //---------
    unsigned GetHistImageHeight() const;
    unsigned GetBinCount() const;
    cv::Scalar GetPlotColor() const;
    cv::Scalar GetAxisColor() const;
    cv::Scalar GetBackgroundColor() const;
    bool GetDrawSpreadOut() const;

    //---------------------
    // Histogram functions
    //---------------------

    // Computes a three channel (BGR) histogram
    void ComputeHistogramBGR(
      const cv::Mat& Image,
      cv::MatND& HistB,
      cv::MatND& HistG,
      cv::MatND& HistR);

    // Computes a single channel (value) histogram
    void ComputeHistogramValue(const cv::Mat& Image, cv::MatND& Hist);

    // Normalizes and draws a three channel (BGR) histogram
    void DrawHistogramBGR(
      cv::MatND& HistB,
      cv::MatND& HistG,
      cv::MatND& HistR,
      cv::Mat& HistImage);

    // Normalizes and draws a single channel histogram
    void DrawHistogramValue(cv::MatND& Hist, cv::Mat& HistImage);

    // Computes and draws a three channel (BGR) histogram
    void ComputeAndDrawHistogramBGR(const cv::Mat& ImageBGR, cv::Mat& ImageHist);

    // Computes and draws a single channel (value) histogram
    void ComputeAndDrawHistogramValue(const cv::Mat& ImageBGR, cv::Mat& ImageHist);

    // General purpose histogram drawing function (does not normalize input)
    void DrawHistogram(
      const cv::Mat& Hist,
      cv::Mat& HistImage);

    // Draws the x axis of the histogram plot
    void DrawHistBar(cv::Mat& HistImage, unsigned BinCount);

    //-------------------------
    // Normalization functions
    //-------------------------
    // Scale the value channel to the max (withouth clipping)
    void NormalizeImageBGR(const cv::Mat& ImageBGR, cv::Mat& ImageBGRNorm);

    // Scale the value channel to a target clipping amount
    void NormalizeClipImageBGR(
        const cv::Mat& ImageBGR,
        cv::Mat& ImageBGRNorm,
        double clipPercent = 2.0f);

  private:
    // Helper functions
    void DrawHistBins(
      const cv::Mat& Hist,
      cv::Mat& HistImage,
      const cv::Scalar& Color);

    void DrawHistBin(
      cv::Mat& HistLayer,
      int value,
      unsigned x,
      const cv::Scalar& Color);

    void DrawHistBin(
      cv::Mat& HistLayer,
      float value,
      unsigned x,
      const cv::Scalar& Color);

    void DrawHistBin(
      cv::Mat& HistLayer,
      double value,
      unsigned x,
      const cv::Scalar& Color);

    unsigned mHistImageHeight;
    unsigned mHistImageBorder;
    unsigned mBinCount;
    unsigned mSpread;
    bool mDrawXAxis;
    bool mDrawSpreadOut;
    cv::Scalar mHistPlotColor;
    cv::Scalar mHistAxisColor;
    cv::Scalar mHistBackgroundColor;
};

#endif //end #ifndef HIST_LIB
