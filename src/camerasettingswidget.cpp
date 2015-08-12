#include "camerasettingswidget.h"
#include <QSpinBox>
#include <QBoxLayout>
#include <QLabel>
#include "utils.h"
#include <QSettings>

class CameraSettingsWidget::Private
{
public:
  Private (CameraSettingsWidget* q );
private:
  CameraSettingsWidget *q;
};

CameraSettingsWidget::Private::Private ( CameraSettingsWidget* q ) : q ( q )
{
}




class CameraSettingWidget : public QWidget {
  Q_OBJECT
public:
  CameraSettingWidget(const Imager::Setting &setting, Imager *imager, QSettings &settings, QWidget* parent = 0);  
};

CameraSettingWidget::CameraSettingWidget(const Imager::Setting& setting, Imager* imager, QSettings& settings, QWidget* parent): QWidget(parent)
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
  connect(spinbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=] (double v) {
    auto s = setting;
    s.value = v;
        imager->setSetting(s);
  });
  connect(imager, &Imager::changed, [=,&settings](const Imager::Setting &changed_setting){
    if(changed_setting.id != setting.id)
      return;
    spinbox->setValue(changed_setting.value);
    settings.beginGroup(imager->name());
    qDebug() << "setting " << changed_setting.name << " to " << changed_setting.value;
    settings.setValue(changed_setting.name,  changed_setting.value);
    settings.endGroup();
  });
}


CameraSettingsWidget::~CameraSettingsWidget()
{
}



CameraSettingsWidget::CameraSettingsWidget(const ImagerPtr& imager, QSettings& settings, QWidget* parent)
  : QWidget(parent), dpointer (this )
{
  setLayout(new QVBoxLayout);
  layout()->setSpacing(2);
  settings.beginGroup(imager->name());
  for(auto setting: imager->settings()) {
    qDebug() << "adding setting: " << setting;
    if(settings.contains(setting.name)) {
      qDebug() << "found setting value for " << setting.name << settings.value(setting.name);
      setting.value = settings.value(setting.name).toDouble();
      imager->setSetting(setting);
    }
    layout()->addWidget(new CameraSettingWidget{setting, imager.get(), settings});
  }
  settings.endGroup();
}

#include "camerasettingswidget.moc"

