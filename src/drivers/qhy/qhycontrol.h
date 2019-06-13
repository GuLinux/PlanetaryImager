#ifndef QHYCONTROL_H
#define QHYCONTROL_H

#include "c++/dptr.h"
#include "qhyccd.h"
#include <QList>
#include "drivers/imager.h"
#include "commons/fwd.h"

FWD_PTR(QHYControl)

class QHYControl
{
public:
    QHYControl(int id, qhyccd_handle *handle);
    ~QHYControl();
    int id() const;
    QString name() const;
    bool isUIControl() const;
    bool available() const;
    static QList<QHYControlPtr> availableControls(qhyccd_handle *handle);
    Imager::Control control() const;
    void reload();
    void setValue(double value);
private:
    DPTR
};

#endif // QHYCONTROL_H
