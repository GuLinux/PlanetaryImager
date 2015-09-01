/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#include "displayimage.h"
#include "configuration.h"
#include <fps_counter.h>
#include <QThread>
#include <QElapsedTimer>
#include <QRect>
#include <QImage>
#include <boost/lockfree/spsc_queue.hpp>
#include <QDebug>
#include "Qt/strings.h"
#include "utils.h"
#include <opencv2/opencv.hpp>

using namespace std;

class DisplayImage::Private {
public:
  Private(Configuration &configuration, DisplayImage *q);
  Configuration &configuration;
  fps_counter capture_fps;
  int milliseconds_limit = 0;
  QElapsedTimer elapsed;
  QRect imageRect;
  bool running = true;
  boost::lockfree::spsc_queue<ImageDataPtr, boost::lockfree::capacity<5>> queue;
  bool detectEdges = false;
  QVector<QRgb> grayScale;
  QImage edgeDetection(QImage &source);

private:
  DisplayImage *q;
};



DisplayImage::Private::Private(Configuration& configuration, DisplayImage* q) : configuration{configuration}, capture_fps{[=](double fps){ emit q->displayFPS(fps);}}, q{q}
{
  for(int i=0; i<0xff; i++)
    grayScale.push_back(qRgb(i, i, i));
}


DisplayImage::~DisplayImage()
{

}

DisplayImage::DisplayImage(Configuration& configuration, QObject* parent)
  : QObject(parent), dptr(configuration, this)
{
  setRecording(false);
}
void DisplayImage::setRecording(bool recording)
{
  int fps = recording ? d->configuration.maxPreviewFPSOnSaving() : 0;
  d->milliseconds_limit = (fps == 0 ? 1000./40. : 1000/fps);
  d->elapsed.restart();
}

void DisplayImage::handle(const ImageDataPtr& imageData)
{
  if( (d->milliseconds_limit > 0 && d->elapsed.elapsed() < d->milliseconds_limit) || !imageData || ! d->queue.push(imageData) ) {
    return;
  }
  d->elapsed.restart();
}

void DisplayImage::create_qimages()
{
  ImageDataPtr imageData;
  while(d->running) {
    if(!d->queue.pop(imageData)) {
      QThread::msleep(1);
      continue;
    }
    ++d->capture_fps;
    auto format = imageData->channels() == 1 ? QImage::Format_Indexed8 : QImage::Format_RGB888;
    cv::Mat origin{imageData->height(), imageData->width(), imageData->channels() == 1 ? CV_8UC1 : CV_8UC3, imageData->data()};
    auto cv_image = new cv::Mat;
    cv::cvtColor(origin, *cv_image, imageData->channels() == 1 ? CV_GRAY2RGB : CV_BGR2RGB);
    cv::imwrite("/tmp/decoded.png", *cv_image);
    QImage image{cv_image->data, cv_image->cols, cv_image->rows, QImage::Format_RGB888, [](void *data){ delete reinterpret_cast<cv::Mat*>(data); }, cv_image};
    if(imageData->channels() == 1) {
      image.setColorTable(d->grayScale);
    }
    d->imageRect = image.rect();
    emit gotImage(d->detectEdges ? d->edgeDetection(image) : image);
  }
  QThread::currentThread()->quit();
}

QRect DisplayImage::imageRect() const
{
  return d->imageRect;
}

void DisplayImage::quit()
{
  d->running = false;
}


void DisplayImage::detectEdges(bool detect)
{
  d->detectEdges = detect;
}

/** Begin qimageblitz copied code
 * As this project seemed to be unmaintained, I simply copied the edge detection algorithm
 * 
  Copyright (C) 1998, 1999, 2001, 2002, 2004, 2005, 2007
 Daniel M. Duley <daniel.duley@verizon.net>
 (C) 2004 Zack Rusin <zack@kde.org>
 (C) 2000 Josef Weidendorfer <weidendo@in.tum.de>
 (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
 (C) 1998, 1999 Dirk Mueller <mueller@kde.org>
 svn://anonsvn.kde.org/home/kde/trunk/kdesupport/qimageblitz
 
 */

#if defined(__i386__) && ( defined(__GNUC__) || defined(__INTEL_COMPILER) )
#  if defined(HAVE_MMX )
#    define USE_MMX_INLINE_ASM
#  endif
#endif
// TODO: reimplement real check
// We always assume we have MMX for now, if that will be a problem, and it's possible to check it at compile time, we'll check for mmx




#define USE_MMX_INLINE_ASM

/*
 * I was looking for a quick way to do edge detection and found
 * the Sobel method. This is a gradient method that applies two 3x3
 * matrixes. These matrixes are:
 *
 * x:  -1, 0, 1      y:  1,  2,  1
 *     -2, 0, 2          0,  0,  0
 *     -1, 0, 1         -1, -2, -1
 *
 * After the matrix is applied you simply calculate the magnitude by
 * |x| + |y|.
 *
 * The one problem w/ this is most descriptions of the algorithm I've
 * seen assume grayscale data so your working with the intensity of the
 * pixel. We do each color channel separately. This is probably wrong,
 * but all the implementations I've seen do this...
 * (mosfet)
 */

// Accumulates the results of applying x and y Sobel masks
#define SOBEL(xm, ym, pixel) \
    xR += qRed((pixel))*(xm); xG += qGreen((pixel))*(xm); \
    xB += qBlue((pixel))*(xm); \
    yR += qRed((pixel))*(ym); yG += qGreen((pixel))*(ym); \
    yB += qBlue((pixel))*(ym);

QImage DisplayImage::Private::edgeDetection(QImage& img)
{


    int x, y, w = img.width(), h = img.height();
    if(w < 3 || h < 3){
        qWarning("Blitz::edge(): Image is too small!");
        return(img);
    }
    if(img.isNull())
        return(img);

    if(img.depth() != 32){
        img = img.convertToFormat(img.hasAlphaChannel() ?
                                  QImage::Format_ARGB32 :
                                  QImage::Format_RGB32);
    }
    else if(img.format() == QImage::Format_ARGB32_Premultiplied)
        img = img.convertToFormat(QImage::Format_ARGB32);

    QImage buffer(w, h, QImage::Format_RGB32);
    QRgb *dest;
    QRgb *s, *scanblock[3];


#ifdef USE_MMX_INLINE_ASM
#ifdef __GNUC__
#warning Using MMX sobel edge
#endif
    if(true){
        int xmatrix[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
        int ymatrix[] = {1, 2, 1, 0, 0, 0, -1, -2, -1};
        int i, *xm, *ym;

        for(y=0; y < h; ++y){
            scanblock[1] = (QRgb *)img.scanLine(y);
            dest = (QRgb *)buffer.scanLine(y);
            if(y == 0){
                scanblock[0] = (QRgb *)img.scanLine(y);
                scanblock[2] = (QRgb *)img.scanLine(y+1);
            }
            else if(y == h-1){
                scanblock[0] = (QRgb *)img.scanLine(y-1);
                scanblock[2]  = (QRgb *)img.scanLine(y);
            }
            else{
                scanblock[0] = (QRgb *)img.scanLine(y-1);
                scanblock[2] = (QRgb *)img.scanLine(y+1);
            }
            //
            // x == 0, double over first pixel
            //
            __asm__ __volatile__
                ("pxor %%mm7, %%mm7\n\t" // used for unpacking
                 "pxor %%mm5, %%mm5\n\t" // clear accumulator
                 "pxor %%mm6, %%mm6\n\t" // ""
                 : : );
            for(i=0, xm=xmatrix, ym=ymatrix; i < 3; ++i, xm+=3, ym+=3){
                s = scanblock[i];
                __asm__ __volatile__
                    (// first pixel
                     "movd (%0), %%mm0\n\t" // load pixel into mm0
                     "punpcklbw %%mm7, %%mm0\n\t" // upgrade to quad
                     "movq %%mm0, %%mm1\n\t" // copy pixel to mm1
                     "movq %%mm0, %%mm4\n\t" // and mm4 since we are doubling over
                     "movd 0(%1), %%mm2\n\t" // load x matrix into mm2
                     "punpckldq %%mm2, %%mm2\n\t" // expand to all words
                     "packssdw %%mm2, %%mm2\n\t"
                     "movd 0(%2), %%mm3\n\t" // load y matrix into mm3
                     "punpckldq %%mm3, %%mm3\n\t" // expand
                     "packssdw %%mm3, %%mm3\n\t"
                     "pmullw %%mm2, %%mm0\n\t" // multiply pixel w/ x matrix
                     "pmullw %%mm3, %%mm1\n\t" // and multiply copy w/ y matrix
                     "paddw %%mm0, %%mm5\n\t"  // add to accumulators
                     "paddw %%mm1, %%mm6\n\t"
                     // second pixel (doubled over)
                     "movq %%mm4, %%mm1\n\t" // copy saved pixel to mm1
                     "movd 4(%1), %%mm2\n\t" // load x matrix into mm2
                     "punpckldq %%mm2, %%mm2\n\t" // expand to all words
                     "packssdw %%mm2, %%mm2\n\t"
                     "movd 4(%2), %%mm3\n\t" // load y matrix into mm3
                     "punpckldq %%mm3, %%mm3\n\t" // expand
                     "packssdw %%mm3, %%mm3\n\t"
                     "pmullw %%mm2, %%mm4\n\t" // multiply pixel w/ x matrix
                     "pmullw %%mm3, %%mm1\n\t" // and multiply copy w/ y matrix
                     "paddw %%mm4, %%mm5\n\t"  // add to accumulators
                     "paddw %%mm1, %%mm6\n\t"
                     // third pixel
                     "movd 4(%0), %%mm0\n\t" // load pixel into mm0
                     "punpcklbw %%mm7, %%mm0\n\t" // upgrade to quad
                     "movq %%mm0, %%mm1\n\t" // copy pixel to mm1
                     "movd 8(%1), %%mm2\n\t" // load x matrix into mm2
                     "punpckldq %%mm2, %%mm2\n\t" // expand to all words
                     "packssdw %%mm2, %%mm2\n\t"
                     "movd 8(%2), %%mm3\n\t" // load y matrix into mm3
                     "punpckldq %%mm3, %%mm3\n\t" // expand
                     "packssdw %%mm3, %%mm3\n\t"
                     "pmullw %%mm2, %%mm0\n\t" // multiply pixel w/ x matrix
                     "pmullw %%mm3, %%mm1\n\t" // and multiply copy w/ y matrix
                     "paddw %%mm0, %%mm5\n\t"  // add to accumulators
                     "paddw %%mm1, %%mm6\n\t"
                     : : "r"(s), "r"(xm), "r"(ym));
            }
            __asm__ __volatile__
                (// calculate abs, sum, and write
                 "movq %%mm5, %%mm0\n\t" // calculate abs of x accumulator
                 "psraw $15, %%mm0\n\t"
                 "pxor %%mm0, %%mm5\n\t"
                 "psubw %%mm0, %%mm5\n\t"
                 "movq %%mm6, %%mm0\n\t" // calculate abs of y accumulator
                 "psraw $15, %%mm0\n\t"
                 "pxor %%mm0, %%mm6\n\t"
                 "psubw %%mm0, %%mm6\n\t"
                 "paddw %%mm5, %%mm6\n\t" // add together
                 "packuswb %%mm6, %%mm6\n\t" // and write
                 "movd %%mm6, (%0)\n\t"
                 : : "r"(dest));
            dest++;

            //
            // Now x == 1, process middle of image
            //

            for(x=1; x < w-1; ++x){
                __asm__ __volatile__
                    ("pxor %%mm5, %%mm5\n\t" // clear accumulator
                     "pxor %%mm6, %%mm6\n\t"
                     : : );
                for(i=0, xm=xmatrix, ym=ymatrix; i < 3; ++i, xm+=3, ym+=3){
                    s = scanblock[i];
                    __asm__ __volatile__
                        (// first pixel
                         "movd (%0), %%mm0\n\t" // load pixel into mm0
                         "punpcklbw %%mm7, %%mm0\n\t" // upgrade to quad
                         "movq %%mm0, %%mm1\n\t" // copy pixel to mm1
                         "movd (%1), %%mm2\n\t" // load x matrix into mm2
                         "punpckldq %%mm2, %%mm2\n\t" // expand to all words
                         "packssdw %%mm2, %%mm2\n\t"
                         "movd (%2), %%mm3\n\t" // load y matrix into mm3
                         "punpckldq %%mm3, %%mm3\n\t" // expand
                         "packssdw %%mm3, %%mm3\n\t"
                         "pmullw %%mm2, %%mm0\n\t" // multiply pixel w/ x matrix
                         "pmullw %%mm3, %%mm1\n\t" // and multiply copy w/ y matrix
                         "paddw %%mm0, %%mm5\n\t"  // add to accumulators
                         "paddw %%mm1, %%mm6\n\t"
                         // second pixel
                         "movd 4(%0), %%mm0\n\t" // load pixel into mm0
                         "punpcklbw %%mm7, %%mm0\n\t" // upgrade to quad
                         "movq %%mm0, %%mm1\n\t" // copy pixel to mm1
                         "movd 4(%1), %%mm2\n\t" // load x matrix into mm2
                         "punpckldq %%mm2, %%mm2\n\t" // expand to all words
                         "packssdw %%mm2, %%mm2\n\t"
                         "movd 4(%2), %%mm3\n\t" // load y matrix into mm3
                         "punpckldq %%mm3, %%mm3\n\t" // expand
                         "packssdw %%mm3, %%mm3\n\t"
                         "pmullw %%mm2, %%mm0\n\t" // multiply pixel w/ x matrix
                         "pmullw %%mm3, %%mm1\n\t" // and multiply copy w/ y matrix
                         "paddw %%mm0, %%mm5\n\t"  // add to accumulators
                         "paddw %%mm1, %%mm6\n\t"
                         // third pixel
                         "movd 8(%0), %%mm0\n\t" // load pixel into mm0
                         "punpcklbw %%mm7, %%mm0\n\t" // upgrade to quad
                         "movq %%mm0, %%mm1\n\t" // copy pixel to mm1
                         "movd 8(%1), %%mm2\n\t" // load x matrix into mm2
                         "punpckldq %%mm2, %%mm2\n\t" // expand to all words
                         "packssdw %%mm2, %%mm2\n\t"
                         "movd 8(%2), %%mm3\n\t" // load y matrix into mm3
                         "punpckldq %%mm3, %%mm3\n\t" // expand
                         "packssdw %%mm3, %%mm3\n\t"
                         "pmullw %%mm2, %%mm0\n\t" // multiply pixel w/ x matrix
                         "pmullw %%mm3, %%mm1\n\t" // and multiply copy w/ y matrix
                         "paddw %%mm0, %%mm5\n\t"  // add to accumulators
                         "paddw %%mm1, %%mm6\n\t"
                         : : "r"(s), "r"(xm), "r"(ym));
                }
                __asm__ __volatile__
                    (// calculate abs, sum, and write
                     "movq %%mm5, %%mm0\n\t" // calculate abs of x accumulator
                     "psraw $15, %%mm0\n\t"
                     "pxor %%mm0, %%mm5\n\t"
                     "psubw %%mm0, %%mm5\n\t"
                     "movq %%mm6, %%mm0\n\t" // calculate abs of y accumulator
                     "psraw $15, %%mm0\n\t"
                     "pxor %%mm0, %%mm6\n\t"
                     "psubw %%mm0, %%mm6\n\t"
                     "paddw %%mm5, %%mm6\n\t" // add together
                     "packuswb %%mm6, %%mm6\n\t" // and write
                     "movd %%mm6, (%0)\n\t"
                     : : "r"(dest));
                dest++;
                ++scanblock[0], ++scanblock[1], ++scanblock[2];
            }

            //
            // x = w-1, double over last pixel
            //

            __asm__ __volatile__
                ("pxor %%mm5, %%mm5\n\t" // clear accumulator
                 "pxor %%mm6, %%mm6\n\t"
                 : : );
            for(i=0, xm=xmatrix, ym=ymatrix; i < 3; ++i, xm+=3, ym+=3){
                s = scanblock[i];
                __asm__ __volatile__
                    (// first pixel
                     "movd (%0), %%mm0\n\t" // load pixel into mm0
                     "punpcklbw %%mm7, %%mm0\n\t" // upgrade to quad
                     "movq %%mm0, %%mm1\n\t" // copy pixel to mm1
                     "movd (%1), %%mm2\n\t" // load x matrix into mm2
                     "punpckldq %%mm2, %%mm2\n\t" // expand to all words
                     "packssdw %%mm2, %%mm2\n\t"
                     "movd (%2), %%mm3\n\t" // load y matrix into mm3
                     "punpckldq %%mm3, %%mm3\n\t" // expand
                     "packssdw %%mm3, %%mm3\n\t"
                     "pmullw %%mm2, %%mm0\n\t" // multiply pixel w/ x matrix
                     "pmullw %%mm3, %%mm1\n\t" // and multiply copy w/ y matrix
                     "paddw %%mm0, %%mm5\n\t"  // add to accumulators
                     "paddw %%mm1, %%mm6\n\t"
                     // second pixel
                     "movd 4(%0), %%mm0\n\t" // load pixel into mm0
                     "punpcklbw %%mm7, %%mm0\n\t" // upgrade to quad
                     "movq %%mm0, %%mm1\n\t" // copy pixel to mm1
                     "movd 4(%1), %%mm2\n\t" // load x matrix into mm2
                     "punpckldq %%mm2, %%mm2\n\t" // expand to all words
                     "packssdw %%mm2, %%mm2\n\t"
                     "movd 4(%2), %%mm3\n\t" // load y matrix into mm3
                     "punpckldq %%mm3, %%mm3\n\t" // expand
                     "packssdw %%mm3, %%mm3\n\t"
                     "pmullw %%mm2, %%mm0\n\t" // multiply pixel w/ x matrix
                     "pmullw %%mm3, %%mm1\n\t" // and multiply copy w/ y matrix
                     "paddw %%mm0, %%mm5\n\t"  // add to accumulators
                     "paddw %%mm1, %%mm6\n\t"
                     // third pixel
                     "movd 4(%0), %%mm0\n\t" // load pixel into mm0
                     "punpcklbw %%mm7, %%mm0\n\t" // upgrade to quad
                     "movq %%mm0, %%mm1\n\t" // copy pixel to mm1
                     "movd 8(%1), %%mm2\n\t" // load x matrix into mm2
                     "punpckldq %%mm2, %%mm2\n\t" // expand to all words
                     "packssdw %%mm2, %%mm2\n\t"
                     "movd 8(%2), %%mm3\n\t" // load y matrix into mm3
                     "punpckldq %%mm3, %%mm3\n\t" // expand
                     "packssdw %%mm3, %%mm3\n\t"
                     "pmullw %%mm2, %%mm0\n\t" // multiply pixel w/ x matrix
                     "pmullw %%mm3, %%mm1\n\t" // and multiply copy w/ y matrix
                     "paddw %%mm0, %%mm5\n\t"  // add to accumulators
                     "paddw %%mm1, %%mm6\n\t"
                     : : "r"(s), "r"(xm), "r"(ym));
            }
            __asm__ __volatile__
                (// calculate abs, sum, and write
                 "movq %%mm5, %%mm0\n\t" // calculate abs of x accumulator
                 "psraw $15, %%mm0\n\t"
                 "pxor %%mm0, %%mm5\n\t"
                 "psubw %%mm0, %%mm5\n\t"
                 "movq %%mm6, %%mm0\n\t" // calculate abs of y accumulator
                 "psraw $15, %%mm0\n\t"
                 "pxor %%mm0, %%mm6\n\t"
                 "psubw %%mm0, %%mm6\n\t"
                 "paddw %%mm5, %%mm6\n\t" // add together
                 "packuswb %%mm6, %%mm6\n\t" // and write
                 "movd %%mm6, (%0)\n\t"
                 : : "r"(dest));
            dest++;
        }
        __asm__ __volatile__ ("emms\n\t" : :);
    }
    else
#endif
    {
        int xR, xG, xB, yR, yG, yB;
        for(y=0; y < h; ++y){
            scanblock[1] = (QRgb *)img.scanLine(y);
            dest = (QRgb *)buffer.scanLine(y);
            if(y == 0){
                scanblock[0] = (QRgb *)img.scanLine(y);
                scanblock[2] = (QRgb *)img.scanLine(y+1);
            }
            else if(y == h-1){
                scanblock[0] = (QRgb *)img.scanLine(y-1);
                scanblock[2]  = (QRgb *)img.scanLine(y);
            }
            else{
                scanblock[0] = (QRgb *)img.scanLine(y-1);
                scanblock[2] = (QRgb *)img.scanLine(y+1);
            }

            // x == 0, double over first pixel
            xR = xG = xB = yR = yG = yB = 0;
            s = scanblock[0];
            SOBEL(-1, 1, *s); SOBEL(0, 2, *s); ++s; SOBEL(1, 1, *s);
            s = scanblock[1];
            SOBEL(-2, 0, *s); SOBEL(0, 0, *s); ++s; SOBEL(2, 0, *s);
            s = scanblock[2];
            SOBEL(-1, -1, *s); SOBEL(0, -2, *s); ++s; SOBEL(1, -1, *s);
            xR = qAbs(xR)+qAbs(yR); xG = qAbs(xG)+qAbs(yG);
            xB = qAbs(xB)+qAbs(yB);
            *dest++ = qRgb(qMin(xR, 255), qMin(xG, 255), qMin(xB, 255));

            // x == 1, process middle of image
            for(x=1; x < w-1; ++x){
                xR = xG = xB = yR = yG = yB = 0;
                s = scanblock[0];
                SOBEL(-1, 1, *s); ++s; SOBEL(0, 2, *s); ++s; SOBEL(1, 1, *s);
                s = scanblock[1];
                SOBEL(-2, 0, *s); ++s; SOBEL(0, 0, *s); ++s; SOBEL(2, 0, *s);
                s = scanblock[2];
                SOBEL(-1, -1, *s); ++s; SOBEL(0, -2, *s); ++s; SOBEL(1, -1, *s);
                ++scanblock[0]; ++scanblock[1]; ++scanblock[2];
                xR = qAbs(xR)+qAbs(yR); xG = qAbs(xG)+qAbs(yG);
                xB = qAbs(xB)+qAbs(yB);
                *dest++ = qRgb(qMin(xR, 255), qMin(xG, 255), qMin(xB, 255));
            }

            // x == w-1, double over last pixel
            xR = xG = xB = yR = yG = yB = 0;
            s = scanblock[0];
            SOBEL(-1, 1, *s); ++s; SOBEL(0, 2, *s); SOBEL(1, 1, *s);
            s = scanblock[1];
            SOBEL(-2, 0, *s); ++s; SOBEL(0, 0, *s); SOBEL(2, 0, *s);
            s = scanblock[2];
            SOBEL(-1, -1, *s); ++s; SOBEL(0, -2, *s); SOBEL(1, -1, *s);
            xR = qAbs(xR)+qAbs(yR); xG = qAbs(xG)+qAbs(yG);
            xB = qAbs(xB)+qAbs(yB);
            *dest++ = qRgb(qMin(xR, 255), qMin(xG, 255), qMin(xB, 255));
        }
    }
    return(buffer);
}

/* End QImageBlitz copied code */
#include "displayimage.moc"