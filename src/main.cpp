#include <QApplication>
#include "planetaryimager_mainwindow.h"
#include "version.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("PlanetaryImager");
    app.setApplicationDisplayName("Planetary Imager");
    app.setApplicationVersion(PLANETARY_IMAGER_VERSION);
    PlanetaryImagerMainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
