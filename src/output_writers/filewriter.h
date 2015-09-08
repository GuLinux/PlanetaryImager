#ifndef FILE_WRITER_H
#define FILE_WRITER_H
#include "imagehandler.h"
#include <QMap>
#include <memory>
#include <functional>
#include <QString>
#include "configuration.h"

class FileWriter : public ImageHandler {
public:
  typedef std::shared_ptr<FileWriter> Ptr;
  typedef std::function<Ptr(const QString &deviceName, Configuration &configuration)> Factory;
  virtual void handle(const cv::Mat& imageData) = 0;
  virtual QString filename() const = 0;
  static QMap<Configuration::SaveFormat, Factory> factories();
};


#endif