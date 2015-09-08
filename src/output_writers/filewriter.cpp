#include "filewriter.h"

#include "serwriter.h"
#ifdef HAVE_CV_VIDEO
#include "cvvideowriter.h"
#endif

using namespace std;
QMap< Configuration::SaveFormat, FileWriter::Factory > FileWriter::factories()
{
  return {
    {Configuration::SER, [](const QString &deviceName, Configuration &configuration){ return make_shared<SERWriter>(deviceName, configuration); }},
#ifdef HAVE_CV_VIDEO
    {Configuration::Video, [](const QString &deviceName, Configuration &configuration){ return make_shared<cvVideoWriter>(deviceName, configuration); }},
#endif
  };
}
