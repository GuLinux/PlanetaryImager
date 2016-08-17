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
#include "cameracontrolswidget.h"
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
#include "ui_cameracontrolswidget.h"
using namespace std;


class CameraControl : public QWidget {
  Q_OBJECT
public:
  CameraControl(const Imager::Control &control, Imager *imager, QSettings &settings, QWidget* parent = 0);
  void apply();
  void restore();
private:
  struct Value {
    double num;
    bool is_auto;
  };

  Imager::Control control;
  Imager *imager;
  Value new_value;
  Value old_value;
  void set_value(Value value);
  ControlWidget *controlWidget;
};

CameraControl::CameraControl(const Imager::Control& control, Imager* imager, QSettings& settings, QWidget* parent)
  : QWidget(parent), control{control}, imager{imager}, new_value{control.value, control.value_auto}, old_value{control.value, control.value_auto}
{
  auto layout = new QHBoxLayout;
  layout->setSpacing(0);
  setLayout(layout);
  layout->addWidget(new QLabel(tr(qPrintable(control.name))));

    if(control.type == Imager::Control::Number) {
      if(control.is_duration)
        controlWidget = new DurationControlWidget;
      else
        controlWidget = new NumberControlWidget;
    }
    else if(control.type == Imager::Control::Combo)
      controlWidget = new MenuControlWidget;
    else if(control.type == Imager::Control::Bool)
      controlWidget = new BoolControlWidget;
    
    controlWidget->update(control);
    layout->addWidget(controlWidget, 1);
    QCheckBox *auto_value = new QCheckBox("auto");
    auto_value->setVisible(control.supports_auto);
    auto_value->setChecked(control.value_auto);
    layout->addWidget(auto_value);
    connect(controlWidget, &ControlWidget::valueChanged, [=](double v) {
      new_value = {v, new_value.is_auto };
    });
    connect(auto_value, &QCheckBox::toggled, this, [this](bool checked) {
      new_value = {new_value.num, checked};
      controlWidget->setEnabled(!checked);
    });

    controlWidget->setEnabled(!control.readonly && ! control.value_auto); // TODO: add different behaviour depending on widget type
    // TODO: handle value
    // TODO: move from here
    connect(imager, &Imager::changed, this, [=,&settings](const Imager::Control &changed_control){
      if(changed_control.id != control.id)
        return;
      qDebug() << "control changed:" << changed_control.id << changed_control.name << "=" << changed_control.value;
      controlWidget->update(changed_control);
    }, Qt::QueuedConnection);
}

void CameraControl::apply()
{
  set_value(new_value);
}

void CameraControl::restore()
{
  set_value(old_value);
}

void CameraControl::set_value(Value value)
{
  if(value.num == control.value && value.is_auto == control.value_auto)
    return;
  qDebug() << value.num;
  old_value = {control.value, control.value_auto};
  control.value = value.num;
  control.value_auto = value.is_auto;
  imager->setControl(control);
  controlWidget->update(control);
}


DPTR_IMPL(CameraControlsWidget)
{
  CameraControlsWidget *q;
  unique_ptr<Ui::CameraControlsWidget> ui;
};



CameraControlsWidget::~CameraControlsWidget()
{
}



CameraControlsWidget::CameraControlsWidget(const ImagerPtr& imager, QSettings& settings, QWidget* parent)
  : QWidget(parent), dptr ( this )
{
  d->ui.reset(new Ui::CameraControlsWidget);
  d->ui->setupUi(this);
  settings.beginGroup(imager->name());
  for(auto imager_control: imager->controls()) {
    qDebug() << "adding setting: " << imager_control;
    if(settings.contains(imager_control.name)) {
      qDebug() << "found setting value for " << imager_control.name << settings.value(imager_control.name);
      imager_control.value = settings.value(imager_control.name).toDouble();
      imager->setControl(imager_control);
    }
    auto control = new CameraControl(imager_control, imager.get(), settings);
    connect(d->ui->apply, &QPushButton::clicked, control, &CameraControl::apply);
    connect(d->ui->restore, &QPushButton::clicked, control, &CameraControl::restore);
    d->ui->controls_box->layout()->addWidget(control);
  }
  settings.endGroup();
  d->ui->controls_box->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
}

#include "cameracontrolswidget.moc"