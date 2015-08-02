#include <QApplication>
#include "planetaryimager_mainwindow.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    PlanetaryImagerMainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
