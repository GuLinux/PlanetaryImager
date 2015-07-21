#ifndef QHYImager_H
#define QHYImager_H

#include <QMainWindow>
#include "qhydriver.h"

class QHYImager : public QMainWindow
{
    Q_OBJECT

public:
    QHYImager();
    virtual ~QHYImager();
private:
  QHYDriver driver;
};

#endif // QHYImager_H
