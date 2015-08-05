#include "camerasettingswidget.h"
#include <QSpinBox>
#include <QBoxLayout>
#include <QLabel>
#include "utils.h"

class CameraSettingsWidget::Private
{
public:
  Private ( const QHYCCDImagerPtr &imager, CameraSettingsWidget *q );
  QHYCCDImagerPtr imager;

private:
  CameraSettingsWidget *q;
};

CameraSettingsWidget::Private::Private ( const QHYCCDImagerPtr &imager, CameraSettingsWidget* q ) : imager(imager), q ( q )
{
}




class CameraSettingWidget : public QWidget {
  Q_OBJECT
public:
  CameraSettingWidget(const QHYCCDImager::Setting &setting, const QHYCCDImagerPtr &imagerPtr, QWidget* parent = 0);  
};

CameraSettingWidget::CameraSettingWidget(const QHYCCDImager::Setting& setting, const QHYCCDImagerPtr& imagerPtr, QWidget* parent): QWidget(parent)
{
  auto layout = new QHBoxLayout;
  setLayout(layout);
  layout->addWidget(new QLabel(tr(qPrintable(setting.name))));
  QDoubleSpinBox *spinbox = new QDoubleSpinBox;
  spinbox->setMinimum(setting.min);
  spinbox->setMaximum(setting.max);
  spinbox->setSingleStep(setting.step != 0 ? setting.step : 0.1);
  spinbox->setValue(setting.value);
  layout->addWidget(spinbox);
  QHYCCDImager *imager = imagerPtr.get();
  connect(spinbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=] (double v) {
    auto s = setting;
    s.value = v;
        imager->setSetting(s);
  });
  connect(imagerPtr.get(), &QHYCCDImager::changed, [=](const QHYCCDImager::Setting &changed_setting){
    if(changed_setting.id != setting.id)
      return;
    spinbox->setValue(changed_setting.value);
  });
}


CameraSettingsWidget::~CameraSettingsWidget()
{
}

CameraSettingsWidget::CameraSettingsWidget ( const QHYCCDImagerPtr &imager, QWidget* parent )
  : QWidget(parent), dpointer (imager, this )
{
  setLayout(new QVBoxLayout);
  layout()->setSpacing(2);
  for(auto setting: imager->settings()) {
    qDebug() << "adding setting: " << setting;
    layout()->addWidget(new CameraSettingWidget{setting, imager});
  }
}

#include "camerasettingswidget.moc"

