#include "camerasettingswidget.h"
#include <QSpinBox>
#include <QBoxLayout>
#include <QLabel>
#include "utils.h"
#include <QSettings>
#include "Qt/functional.h"
#include <QLayout>
#include <QCheckBox>
#include <QComboBox>

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


class SettingWidget : public QWidget {
  Q_OBJECT
public:
  SettingWidget(QWidget* parent = 0) : QWidget(parent) {
    setLayout(new QVBoxLayout);
    layout()->setMargin(0);
    layout()->setSpacing(0);
  }
public slots:
  virtual void update(const Imager::Setting &setting) = 0;
signals:
  void valueChanged(double value);
};

class NumberSettingWidget : public SettingWidget {
  Q_OBJECT
public:
    NumberSettingWidget(QWidget* parent = 0);
public slots:
  virtual void update(const Imager::Setting &setting);
private:
  QDoubleSpinBox *edit;
};

NumberSettingWidget::NumberSettingWidget(QWidget* parent): SettingWidget(parent)
{
  layout()->addWidget(edit = new QDoubleSpinBox);
  connect(edit, F_PTR(QDoubleSpinBox, valueChanged, double), this, &SettingWidget::valueChanged);
}

void NumberSettingWidget::update(const Imager::Setting& setting)
{
  edit->setMinimum(setting.min);
  edit->setMaximum(setting.max);
  edit->setSingleStep(setting.step != 0 ? setting.step : 0.1);
  edit->setValue(setting.value);
}

class BooleanSettingWidget : public SettingWidget {
  Q_OBJECT
public:
    BooleanSettingWidget(QWidget* parent = 0);
public slots:
  virtual void update(const Imager::Setting &setting);
private:
  QCheckBox *edit;
};

BooleanSettingWidget::BooleanSettingWidget(QWidget* parent): SettingWidget(parent)
{
  layout()->addWidget(edit = new QCheckBox);
  connect(edit, &QCheckBox::toggled, [=](bool checked) { emit valueChanged(checked ? 1 : 0); });
}

void BooleanSettingWidget::update(const Imager::Setting& setting)
{
  edit->setChecked(setting.value == 1);
}

class MenuSettingWidget : public SettingWidget {
  Q_OBJECT
public:
    MenuSettingWidget(QWidget* parent = 0);
public slots:
  virtual void update(const Imager::Setting &setting);
private:
  QComboBox *edit;
};



MenuSettingWidget::MenuSettingWidget(QWidget* parent): SettingWidget(parent)
{
  layout()->addWidget(edit = new QComboBox);
  connect(edit, F_PTR(QComboBox, currentIndexChanged, int), [=](int index) { emit valueChanged(index); });
}

void MenuSettingWidget::update(const Imager::Setting& setting)
{
  edit->clear();
  for(auto item: setting.choices)
    edit->addItem(item);
  edit->setCurrentIndex(setting.value);
}



class CameraSettingWidget : public QWidget {
  Q_OBJECT
public:
  CameraSettingWidget(const Imager::Setting &setting, Imager *imager, QSettings &settings, QWidget* parent = 0);  
};

CameraSettingWidget::CameraSettingWidget(const Imager::Setting& setting, Imager* imager, QSettings& settings, QWidget* parent): QWidget(parent)
{
  auto layout = new QHBoxLayout;
  layout->setSpacing(0);
  setLayout(layout);
  layout->addWidget(new QLabel(tr(qPrintable(setting.name))));

    SettingWidget *settingWidget;
    if(setting.type == Imager::Setting::Number)
      settingWidget = new NumberSettingWidget;
    else if(setting.type == Imager::Setting::Combo)
      settingWidget = new MenuSettingWidget;
    else if(setting.type == Imager::Setting::Bool)
      settingWidget = new BooleanSettingWidget;
    
    settingWidget->update(setting);
    layout->addWidget(settingWidget);
    connect(settingWidget, &SettingWidget::valueChanged, [=](double v) {
      qDebug() << v;
      auto s = setting;
      s.value = v;
      imager->setSetting(s);
    });
    connect(imager, &Imager::changed, [=,&settings](const Imager::Setting &changed_setting){
      if(changed_setting.id != setting.id)
	return;
      settingWidget->update(changed_setting);
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
  : QWidget(parent), dptr (this )
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
  layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

#include "camerasettingswidget.moc"

