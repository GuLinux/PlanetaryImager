#ifndef CAMERASETTINGSWIDGET_H
#define CAMERASETTINGSWIDGET_H

#include <QWidget>
#include "dptr.h"
#include "qhyccdimager.h"

class CameraSettingsWidget : public QWidget
{
public:
~CameraSettingsWidget();
CameraSettingsWidget(const QHYCCDImagerPtr &imager, QWidget* parent = 0);

private:
    D_PTR
};

#endif // CAMERASETTINGSWIDGET_H
