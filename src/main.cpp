#include <QApplication>
#include "qhymainwindow.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QHYMainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
