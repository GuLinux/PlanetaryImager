#include <QApplication>
#include "planetaryimager_mainwindow.h"
#include "version.h"
#include <iostream>
#include <iomanip>
using namespace std;

void log_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  static map<QtMsgType, string> log_levels {
    {QtFatalMsg   , "FATAL"},
    {QtCriticalMsg, "CRITICAL"},
    {QtWarningMsg , "WARNING"},
    {QtDebugMsg   , "DEBUG"},
  };
  QString position;
  if(context.file && context.line) {
    position = QString("%1:%2").arg(context.file).arg(context.line).replace(SRC_DIR, "");
  }
  QString function = context.function ? context.function : "";
  cerr << setw(8) << log_levels[type] << " - " /*<< qPrintable(position) << "@"*/<< qPrintable(function) << " " << qPrintable(msg) << endl;
  cerr.flush();
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    qInstallMessageHandler(log_handler);
    app.setApplicationName("PlanetaryImager");
    app.setApplicationDisplayName("Planetary Imager");
    app.setApplicationVersion(PLANETARY_IMAGER_VERSION);
    PlanetaryImagerMainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}
