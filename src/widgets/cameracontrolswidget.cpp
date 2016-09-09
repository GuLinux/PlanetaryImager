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
#include "configuration.h"
#include <QSettings>
#include "Qt/functional.h"
#include <QLayout>
#include <QCheckBox>
#include <QComboBox>
#include "controls/controls.h"
#include "ui_cameracontrolswidget.h"
#include "Qt/strings.h"

using namespace std;
using namespace std::placeholders;


class CameraControl : public QObject {
  Q_OBJECT
public:
  CameraControl(const Imager::Control &control, Imager *imager, QWidget* parent = 0);
  void apply();
  void restore();
  QString label() const;
  QCheckBox *autoValueWidget() const;
  ControlWidget *controlWidget() const;
  void control_updated(const Imager::Control &changed_control);
  bool is_pending() const;
private:

  Imager::Control control;
  Imager::Control new_value;
  Imager *imager;
  void set_value(const Imager::Control & value);
  ControlWidget *control_widget;
  QCheckBox *auto_value_widget;
signals:
    void changed();
};

CameraControl::CameraControl(const Imager::Control& control, Imager* imager, QWidget* parent)
  : QObject(parent), control{control}, new_value{control}, imager{imager}
{
    if(control.type == Imager::Control::Number) {
      if(control.is_duration)
        control_widget = new DurationControlWidget;
      else
        control_widget = new NumberControlWidget;
    }
    else if(control.type == Imager::Control::Combo)
      control_widget = new MenuControlWidget;
    else if(control.type == Imager::Control::Bool)
      control_widget = new BoolControlWidget;
    
    control_widget->update(control);

    auto_value_widget = new QCheckBox("auto");
    auto_value_widget->setVisible(control.supports_auto);
    auto_value_widget->setChecked(control.value_auto);
    
    connect(control_widget, &ControlWidget::valueChanged, [=](double v) {
      new_value.value = v;
      emit changed();
    });
    connect(auto_value_widget, &QCheckBox::toggled, this, [this](bool checked) {
      new_value.value_auto = checked;
      control_widget->setEnabled(!checked);
      emit changed();
    });

    control_widget->setEnabled(!control.readonly && ! control.value_auto); // TODO: add different behaviour depending on widget type
    // TODO: handle value
    // TODO: move from here
    connect(imager, &Imager::changed, this, &CameraControl::control_updated, Qt::QueuedConnection);
}

void CameraControl::control_updated(const Imager::Control& changed_control)
{
      if(changed_control.id != control.id)
        return;
      bool is_expected_value = changed_control.same_value(new_value);
      qDebug() << "control changed:" << changed_control << "(" << control << "/" << new_value << "), is_expected_value: " << is_expected_value;
      control = changed_control;
      new_value = control;
      control_widget->update(changed_control);
      control_widget->setStyleSheet("background: %1;"_q % (is_expected_value ? "green" : "red") );
      QTimer::singleShot(5000, [this]{ control_widget->setStyleSheet(""); });
      emit changed();
}

QString CameraControl::label() const
{
  return tr(qPrintable(control.name));
}

QCheckBox* CameraControl::autoValueWidget() const
{
  return auto_value_widget;
}

ControlWidget* CameraControl::controlWidget() const
{
  return control_widget;
}


void CameraControl::apply()
{
  set_value(new_value);
}

void CameraControl::restore()
{
  set_value(control);
}

void CameraControl::set_value(const Imager::Control &value)
{
  if(control.same_value(value))
    return;
  qDebug() << "GUI: setting control " << control << " to " << value;;
  imager->setControl(value);
  control_widget->update(value);
}

bool CameraControl::is_pending() const
{
    return ! control.same_value(new_value);
}



DPTR_IMPL(CameraControlsWidget)
{
  CameraControlsWidget *q;
  unique_ptr<Ui::CameraControlsWidget> ui;
  list<CameraControl *> control_widgets;
  void controls_changed();
};



CameraControlsWidget::~CameraControlsWidget()
{
}



CameraControlsWidget::CameraControlsWidget(Imager *imager, Configuration &configuration, QWidget *parent) : dptr(this)
{
  d->ui.reset(new Ui::CameraControlsWidget);
  d->ui->setupUi(this);
  auto grid = new QGridLayout(d->ui->controls_box);
  int row = 0;
  for(auto imager_control: imager->controls()) {
    qDebug() << "adding setting: " << imager_control;
    auto control = new CameraControl(imager_control, imager, this);
    d->control_widgets.push_back(control);
    connect(control, &CameraControl::changed, this, bind(&Private::controls_changed, d.get()));
    connect(d->ui->apply, &QPushButton::clicked, control, &CameraControl::apply);
    connect(d->ui->restore, &QPushButton::clicked, control, &CameraControl::restore);
    grid->addWidget(new QLabel(control->label()), row, 0);
    grid->addWidget(control->controlWidget(), row, 1);
    grid->addWidget(control->autoValueWidget(), row++, 2);
  }
  grid->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Expanding), row, 0, 3);
  grid->setRowStretch(row, 1);
  grid->setColumnStretch(1, 1);
}

void CameraControlsWidget::Private::controls_changed()
{
    bool any_changed = std::any_of(control_widgets.begin(), control_widgets.end(), bind(&CameraControl::is_pending, _1));
    qDebug() << "Any control pending: " << any_changed;
    ui->apply->setEnabled(any_changed);
    ui->restore->setEnabled(any_changed);
}

#include "cameracontrolswidget.moc"
