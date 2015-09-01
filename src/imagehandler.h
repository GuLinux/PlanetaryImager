#ifndef IMAGE_HANDLER_H
#define IMAGE_HANDLER_H

#include "image_data.h"
#include <memory>
#include <QList>
#include<algorithm>
#include <opencv2/opencv.hpp>


class ImageHandler {
public:
  virtual void handle(const cv::Mat &imageData) = 0;
};

typedef std::shared_ptr<ImageHandler> ImageHandlerPtr;

class ImageHandlers : public ImageHandler {
public:
  ImageHandlers(std::initializer_list<ImageHandlerPtr> handlers) : handlers{handlers} {}
  virtual void handle(const cv::Mat& imageData) {
    for(auto handler: handlers)
      handler->handle(imageData);
  }
private:
  QList<ImageHandlerPtr> handlers;
};
#endif
