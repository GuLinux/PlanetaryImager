/*
 * Copyright (C) 2016  Marco Gulino <marco@gulinux.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
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
#include "controls/controls.h"
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



class CameraSettingWidget : public QWidget {
  Q_OBJECT
public:
  CameraSettingWidget(const Imager::Setting &setting, Imager *imager, QSettings &settings, QWidget* parent = 0);
  void apply();
  void restore();
private:
  struct Value {
    bool num;
    bool is_auto;
  };

  Imager::Setting setting;
  Imager *imager;
  Value new_value;
  Value old_value;
  void set_value(Value value);
  SettingWidget *settingWidget;
};

CameraSettingWidget::CameraSettingWidget(const Imager::Setting& setting, Imager* imager, QSettings& settings, QWidget* parent)
  : QWidget(parent), setting{setting}, imager{imager}, new_value{setting.value, setting.value_auto}, old_value{setting.value, setting.value_auto}
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
    layout->addWidget(settingWidget, 1);
    QCheckBox *auto_value = new QCheckBox("auto");
    auto_value->setVisible(setting.supports_auto);
    auto_value->setChecked(setting.value_auto);
    layout->addWidget(auto_value);
    connect(settingWidget, &SettingWidget::valueChanged, [=](double v) {
      new_value = {v, new_value.is_auto };
    });
    connect(auto_value, &QCheckBox::toggled, this, [this](bool checked) {
      new_value = {new_value.num, checked};
    });

    settingWidget->setEnabled(!setting.readonly); // TODO: add different behaviour depending on widget type
    // TODO: handle value
    // TODO: move from here
    connect(imager, &Imager::changed, this, [=,&settings](const Imager::Setting &changed_setting){
      if(changed_setting.id != setting.id)
        return;
      qDebug() << "setting changed:" << changed_setting.id << changed_setting.name << "=" << changed_setting.value;
      settingWidget->update(changed_setting);
    }, Qt::QueuedConnection);
}

void CameraSettingWidget::apply()
{
  set_value(new_value);
}

void CameraSettingWidget::restore()
{
  set_value(old_value);
}

void CameraSettingWidget::set_value(Value value)
{
  if(value.num == setting.value && value.is_auto == setting.value_auto)
    return;
  qDebug() << value.num;
  old_value = {setting.value, setting.value_auto};
  setting.value = value.num;
  setting.value_auto = value.is_auto;
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

