#include <QtGui/QApplication>
#include "QHYImager.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QHYImager qhyimager;
    qhyimager.show();
    return app.exec();
}
