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
#include "ui_camerasettingswidget.h"
using namespace std;

class CameraSettingsWidget::Private
{
public:
  Private (CameraSettingsWidget* q );
  unique_ptr<Ui::CameraSettingsWidget> ui;
private:
  CameraSettingsWidget *q;
};

CameraSettingsWidget::Private::Private ( CameraSettingsWidget* q ) : ui{new Ui::CameraSettingsWidget}, q ( q )
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
  connect(edit, F_PTR(QComboBox, currentIndexChanged, int), [=](int index) { emit valueChanged(edit->itemData(index).toDouble()); });
}

void MenuSettingWidget::update(const Imager::Setting& setting)
{
//   if(edit->currentData().toDouble() == setting.value)
//     return;
  edit->clear();
  for(auto item: setting.choices) {
    edit->addItem(item.label, item.value);
  }
  edit->setCurrentIndex(edit->findData(setting.value));
}



class CameraSettingWidget : public QWidget {
  Q_OBJECT
public:
  CameraSettingWidget(const Imager::Setting &setting, Imager *imager, QSettings &settings, QWidget* parent = 0);
  void apply();
  void restore();
private:
  Imager::Setting setting;
  Imager *imager;
  double new_value;
  double old_value;
  void set_value(double value);
  SettingWidget *settingWidget;
signals:
  void value_changed(double);
};

CameraSettingWidget::CameraSettingWidget(const Imager::Setting& setting, Imager* imager, QSettings& settings, QWidget* parent)
  : QWidget(parent), setting{setting}, imager{imager}, new_value{setting.value}, old_value{setting.value}
{
  auto layout = new QHBoxLayout;
  layout->setSpacing(0);
  setLayout(layout);
  layout->addWidget(new QLabel(tr(qPrintable(setting.name))));

    if(setting.type == Imager::Setting::Number)
      settingWidget = new NumberSettingWidget;
    else if(setting.type == Imager::Setting::Combo)
      settingWidget = new MenuSettingWidget;
    else if(setting.type == Imager::Setting::Bool)
      settingWidget = new BooleanSettingWidget;
    
    settingWidget->update(setting);
    layout->addWidget(settingWidget);
    connect(settingWidget, &SettingWidget::valueChanged, [=](double v) {
      new_value = v;
    });
    // TODO: move from here
    connect(imager, &Imager::changed, [=,&settings](const Imager::Setting &changed_setting){
      if(changed_setting.id != setting.id)
	return;
      qDebug() << "setting changed:" << changed_setting.id << changed_setting.name << "=" << changed_setting.value;
      settingWidget->update(changed_setting);
    });
}

void CameraSettingWidget::apply()
{
  set_value(new_value);
}

void CameraSettingWidget::restore()
{
  set_value(old_value);
}

void CameraSettingWidget::set_value(double value)
{
  if(value == setting.value)
    return;
  qDebug() << value;
  old_value = setting.value;
  setting.value = value;
  imager->setSetting(setting);
  settingWidget->update(setting);
}


CameraSettingsWidget::~CameraSettingsWidget()
{
}



CameraSettingsWidget::CameraSettingsWidget(const ImagerPtr& imager, QSettings& settings, QWidget* parent)
  : QWidget(parent), dptr (this )
{
  d->ui->setupUi(this);
  settings.beginGroup(imager->name());
  for(auto setting: imager->settings()) {
    qDebug() << "adding setting: " << setting;
    if(settings.contains(setting.name)) {
      qDebug() << "found setting value for " << setting.name << settings.value(setting.name);
      setting.value = settings.value(setting.name).toDouble();
      imager->setSetting(setting);
    }
    auto setting_widget = new CameraSettingWidget{setting, imager.get(), settings};
    connect(d->ui->apply, &QPushButton::clicked, setting_widget, &CameraSettingWidget::apply);
    connect(d->ui->restore, &QPushButton::clicked, setting_widget, &CameraSettingWidget::restore);
    d->ui->settings_box->layout()->addWidget(setting_widget);
  }
  settings.endGroup();
  d->ui->settings_box->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
}

#include "camerasettingswidget.moc"

