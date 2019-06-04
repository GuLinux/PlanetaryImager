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

#include "simulatorimager.h"
#include "c++/stlutils.h"
#include <QThread>
#include <QFile>
#include "commons/utils.h"
#include "commons/opencv_utils.h"
#include <QTimer>
#include <QRect>
#include "drivers/imagerthread.h"
#include "drivers/roi.h"
#include "commons/frame.h"

using namespace std;
using namespace std::chrono_literals;
using namespace GuLinux;

typedef QMap<QString, Imager::Control> SimulatorSettings;
class SimulatorImagerWorker;
DPTR_IMPL(SimulatorImager) {
  SimulatorSettings settings;
  shared_ptr<SimulatorImagerWorker> worker;

  LOG_C_SCOPE(SimulatorImager);
  ROIValidator::ptr roi_validator;
};

class SimulatorImagerWorker : public ImagerThread::Worker {
public:
  SimulatorImagerWorker(SimulatorSettings &settings);
  
  FramePtr shoot() override;
  void setROI(const QRect &roi);
  enum ImageType{ BGR = 0, Mono = 10, Bayer = 20};
  QRect ROI() const { return roi; }
private:
  SimulatorSettings &settings;
  QHash<int, cv::Mat> images;
  QRect roi;
  LOG_C_SCOPE(SimulatorImagerWorker);
};

Q_DECLARE_METATYPE(SimulatorImagerWorker::ImageType)

SimulatorImager::SimulatorImager(const ImageHandler::ptr& handler) : Imager(handler), dptr()
{
  d->roi_validator = make_shared<ROIValidator>(list<ROIValidator::Rule>{
    ROIValidator::x_multiple(2),
    ROIValidator::y_multiple(2),
    ROIValidator::width_multiple(4),
    ROIValidator::height_multiple(2),
    });
  d->settings = {
    {"exposure",    Control{1l, "exposure"}.set_range(0.1, 1000., 0.1).set_value(19.5).set_is_duration(true).set_is_exposure(true).set_duration_unit(1ms) },
    {"movement", Control{2, "movement"}.set_range(0., 5., 1.).set_value(1.)},
    {"seeing",   Control{3l, "seeing"}.set_range(0, 5, 1).set_value(1).set_supports_auto(true)},
    {"bin",	 Control{4l, "bin", Control::Combo}.set_value(4).add_choice("1x1", 1).add_choice("2x2", 2).add_choice("3x3", 3).add_choice("4x4", 4) }, 
    {"format", Control{5l, "format", Control::Combo}.set_value(static_cast<int>(SimulatorImagerWorker::BGR))
      .add_choice("Mono", static_cast<int>(SimulatorImagerWorker::Mono))
      .add_choice("BGR", static_cast<int>(SimulatorImagerWorker::BGR))
      .add_choice("Bayer", static_cast<int>(SimulatorImagerWorker::Bayer))
    }, 
    {"bpp",	 Control{6l, "bpp", Control::Combo}.set_value(8).add_choice("8", 8).add_choice("16", 16)}, 
    {"reject",	 Control{7l, "reject", Control::Combo}.set_value(0).add_choice("Never", 0).add_choice("1 out of 10", 10).add_choice("1 out of 5", 5).add_choice("1 out of 3", 3).add_choice("1 out of 2", 2)}, 
    {"max_speed",	 Control{8l, "max_speed", Control::Bool}.set_value(false) }, 
  };
  qDebug() << "Max speed: " << d->settings["max_speed"];
}

SimulatorImager::~SimulatorImager()
{
}


Imager::Properties SimulatorImager::properties() const
{
    // Simulating ASI 178mm chip: 2.4x2.4 um pixels, sensor size 7.4x5mm, resolution 3096x2080
  // return Imager::Chip().set_pixelsize_chipsize(2.4, 2.4, 7.4, 5);
  // return Imager::Chip().set_resolution_chipsize({3096, 2080}, 7.4, 5);
  return Imager::Properties().set_resolution_pixelsize({3096, 2080}, 2.4, 2.4) << ROI << LiveStream << Temperature;
}

QString SimulatorImager::name() const
{
  return "Simulator Imager";
}

void SimulatorImager::setControl(const Imager::Control& setting)
{
  wait_for(push_job_on_thread([=]{
    static uint64_t controls_changed = 0;
    qDebug() << "Received control: " << setting << "; saved: " << d->settings[setting.name];
    int reject_every = d->settings["reject"].value.toInt();
    if(reject_every > 0 && controls_changed++ % reject_every == 0) {
        qDebug() << "Rejecting control (" << reject_every << " reached)";
        emit changed(d->settings[setting.name]);
        return;
    }
    d->settings[setting.name] = setting;
    emit changed(setting);
  }));
}

void SimulatorImager::readTemperature()
{
    push_job_on_thread([&]{
       double celsius = SimulatorImager::rand(200, 500) / 10.;
       emit temperature(celsius);
     });
}

Imager::Controls SimulatorImager::controls() const
{
  return d->settings.values();
}


int SimulatorImager::rand(int a, int b)
{
   return qrand() % ((b + 1) - a) + a;
}

SimulatorImagerWorker::SimulatorImagerWorker(SimulatorSettings &settings) : settings{settings}
{
    QFile file(":/simulator/jupiter_hubble.jpg");
    file.open(QIODevice::ReadOnly);
    QByteArray file_data = file.readAll();
    images[BGR] = cv::imdecode(cv::InputArray{file_data.data(), file_data.size()}, cv::IMREAD_COLOR);
    enum ChannelIndexes {B = 0, G = 1, R = 2 };
    cv::cvtColor(images[BGR], images[Mono], cv::COLOR_BGR2GRAY);
    images[Bayer] = cv::Mat(images[BGR].rows, images[BGR].cols, CV_8UC1);
    
    QHash<QPair<int,int>, int> bayer_pattern_channels {
      // B=0, G=1, R=2
      { {0, 0}, R }, { {0, 1}, G }, { {1, 0}, G }, { {1, 1}, B}
    };
    for(int row = 0; row < images[BGR].rows; row++) {
      for(int column = 0; column < images[BGR].cols; column++) {
        int channel = bayer_pattern_channels[{row%2, column%2}];
        images[Bayer].at<uint8_t>(row, column) = images[BGR].at<cv::Vec3b>(row, column).val[channel];
      }
    }
    for(int bin = 1; bin < 5; bin++) {
      double ratio = 4. / bin;
      cv::resize(images[BGR], images[BGR + bin], {}, ratio, ratio);
      cv::resize(images[Mono], images[Mono + bin], {}, ratio, ratio);
    }
}


FramePtr SimulatorImagerWorker::shoot()
{
  static map<SimulatorImagerWorker::ImageType, Frame::ColorFormat> formats {
    {Mono, Frame::Mono},
    {BGR, Frame::BGR},
    {Bayer, Frame::Bayer_RGGB},
  };
  auto rand = [](int a, int b) { return qrand() % ((b + 1) - a) + a; };
  cv::Mat cropped, blurred, result;
  Imager::Control exposure, seeing, bpp;
  exposure = settings["exposure"];
  auto format = static_cast<ImageType>(settings["format"].get_value<int>());
  seeing = settings["seeing"];
  bpp = settings["bpp"];
  
  bool is_bayer = format == Bayer;
  const cv::Mat &image = is_bayer ? images[Bayer] : images[format + settings["bin"].get_value<int>()];
  int h = image.rows;
  int w = image.cols;
  int crop_factor = settings["movement"].get_value<int>();
  int pix_w = rand(0, crop_factor);
  int pix_h = rand(0, crop_factor);

  cv::Rect crop_rect(0, 0, w, h);
  crop_rect -= cv::Size{crop_factor, crop_factor};
  crop_rect += cv::Point{pix_w, pix_h};
  cropped = is_bayer ? image : image(crop_rect);
  if(roi.isValid()) {
    cropped = cropped(cv::Rect{roi.x(), roi.y(), roi.width(), roi.height()});
  }
  if(! is_bayer && (rand(0, seeing.range.max.toInt()) > (seeing.value_auto ? 3 : seeing.value.toInt()) ) ) {
      auto ker_size = rand(1, 7);
      cv::blur(cropped, blurred, {ker_size, ker_size});
  } else {
      cropped.copyTo(blurred);
  }
  double exposure_percent = (exposure.value.toDouble() - exposure.range.min.toDouble()) / (exposure.range.max.toDouble()- exposure.range.min.toDouble()) * 100.;
  double exposure_offset = log(exposure_percent)*150 - 100;

  result = blurred + cv::Scalar{exposure_offset, exposure_offset, exposure_offset};

  if(bpp.value == 16)
      result.convertTo(result, result.channels() == 1 ? CV_16UC1 : CV_16UC3, BITS_8_TO_16);
  auto frame_format = formats[format];
  auto frame = make_shared<Frame>( bpp.value.toInt(), frame_format, QSize{result.cols, result.rows} );
  move(result.data, result.data + frame->size(), frame->data());
  if(settings["max_speed"].get_value<bool>())
    return frame;
  Scope sleep{[=]{
    QThread::usleep(exposure.value.toDouble() * 1000.);
  } };
  return frame;
}

void SimulatorImager::clearROI()
{
  d->worker->setROI({});
}

void SimulatorImager::setROI(const QRect &roi)
{
  d->worker->setROI(d->roi_validator->validate(roi, d->worker->ROI()));
}

void SimulatorImagerWorker::setROI(const QRect& roi)
{
  this->roi = roi;
}



void SimulatorImager::startLive()
{
  restart([&] { return d->worker = make_shared<SimulatorImagerWorker>(d->settings); });
}
