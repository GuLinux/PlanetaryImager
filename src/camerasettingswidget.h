#ifndef CAMERASETTINGSWIDGET_H
#define CAMERASETTINGSWIDGET_H

#include <QWidget>
#include "dptr.h"
#include "qhyccdimager.h"

class QSettings;
class CameraSettingsWidget : public QWidget
{
public:
~CameraSettingsWidget();
CameraSettingsWidget(const QHYCCDImagerPtr &imager, QSettings &settings, QWidget* parent = 0);

private:
    D_PTR
};

#endif // CAMERASETTINGSWIDGET_H
