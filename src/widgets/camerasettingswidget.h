#ifndef CAMERASETTINGSWIDGET_H
#define CAMERASETTINGSWIDGET_H

#include <QWidget>
#include "dptr.h"
#include "drivers/imager.h"

class QSettings;
class CameraSettingsWidget : public QWidget
{
public:
~CameraSettingsWidget();
CameraSettingsWidget(const ImagerPtr &imager, QSettings &settings, QWidget* parent = 0);

private:
    DPTR
};

#endif // CAMERASETTINGSWIDGET_H
