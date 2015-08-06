#include <QApplication>
#include "planetaryimager_mainwindow.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("PlanetaryImager");
    app.setApplicationDisplayName("Planetary Imager");
    app.setApplicationVersion("0.1");
    PlanetaryImagerMainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
