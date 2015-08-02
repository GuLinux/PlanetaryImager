#ifndef IMAGE_HANDLER_H
#define IMAGE_HANDLER_H

#include "image_data.h"
#include <memory>
class ImageHandler {
public:
  virtual void handle(const ImageDataPtr &imageData) = 0;
};

typedef std::shared_ptr<ImageHandler> ImageHandlerPtr;
#endif
