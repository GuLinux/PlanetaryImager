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
#include "commons/utils.h"
#include "commons/configuration.h"
#include <QSettings>
#include "Qt/functional.h"
#include <QLayout>
#include <QCheckBox>
#include <QComboBox>
#include "controls/controls.h"
#include "ui_cameracontrolswidget.h"
#include "Qt/strings.h"
#include <QMetaObject>
#include <QMenu>
#include <QAction>
#include <QStringListModel>
#include <QInputDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonDocument>
#include "Qt/functional.h"
#include <QSortFilterProxyModel>

using namespace std;
using namespace std::placeholders;


class CameraControl : public QObject {
  Q_OBJECT
public:
  CameraControl(const Imager::Control &control, Imager *imager, QWidget* parent = 0);
  void apply();
  void restore();
  QString label() const;
  QCheckBox *autoValueWidget() const { return auto_value_widget; }
  QCheckBox *onOffValueWidget() const { return on_off_value_widget; }
  QLabel *controlChangedLed() const { return control_changed_led; }
  ControlWidget *controlWidget() const { return control_widget; }
  void control_updated(const Imager::Control &changed_control);
  bool is_pending() const;
  void importing(const QVariantList &controls);
private:
  
  Imager::Control control;
  Imager::Control new_value;
  Imager *imager;
  void set_value(const Imager::Control & value);
  ControlWidget *control_widget;
  QCheckBox *auto_value_widget;
  QCheckBox *on_off_value_widget;
  QLabel *control_changed_led;
private slots:
  void auto_changed(bool isAuto);
  void on_off_changed(bool isOn);
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
  control_changed_led = new QLabel();
  control_changed_led->setHidden(true);
  
  auto_value_widget = new QCheckBox("auto");
  auto_value_widget->setVisible(control.supports_auto);
  auto_value_widget->setChecked(control.value_auto);

  on_off_value_widget = new QCheckBox("on");
  on_off_value_widget->setVisible(control.supports_onOff);
  on_off_value_widget->setChecked(control.value_onOff);
  
  connect(control_widget, &ControlWidget::valueChanged, [=](const QVariant &v) {
    new_value.value = v;
    emit changed();
  });
  connect(auto_value_widget, &QCheckBox::toggled, this, &CameraControl::auto_changed);
  connect(on_off_value_widget, &QCheckBox::toggled, this, &CameraControl::on_off_changed);
  
  control_widget->setEnabled(!control.readonly && ! control.value_auto && !(control.supports_onOff && !control.value_onOff));
  connect(imager, &Imager::changed, this, &CameraControl::control_updated, Qt::QueuedConnection);
}

void CameraControl::auto_changed(bool isAuto)
{
  new_value.value_auto = isAuto;
  if(! isAuto)
    new_value.value = control_widget->value();
  control_widget->setEnabled(!isAuto);
  auto_value_widget->setChecked(isAuto);
  emit changed();
}

void CameraControl::on_off_changed(bool isOn)
{
    new_value.value_onOff = isOn;
    control_widget->setEnabled(isOn && !auto_value_widget->isChecked());
    auto_value_widget->setEnabled(isOn);
    on_off_value_widget->setChecked(isOn);
    emit changed();
}

void CameraControl::importing(const QVariantList& controls)
{
  auto found = find_if(controls.begin(), controls.end(), [this](const QVariant &v){ return v.toMap()["id"].toLongLong() == control.id; });
  if(found == controls.end())
    return;
  new_value.value = found->toMap()["value"];
  new_value.value_auto = found->toMap()["auto"].toBool();
}


void CameraControl::control_updated(const Imager::Control& changed_control)
{
  static QPixmap red_dot{":/resources/dot_red.png"};
  static QPixmap green_dot{":/resources/dot_green.png"};
  if(changed_control.id != control.id)
    return;
  bool is_expected_value = changed_control.same_value(new_value);
  qDebug() << "control changed: incoming =" << changed_control;
  qDebug() << "control changed: expected =" << new_value;
  qDebug() << "control changed: old value=" << control;
  qDebug() << "control changed: is same value: " << is_expected_value;
  control = changed_control;
  new_value = control;
  control_widget->update(changed_control);
  if(changed_control.supports_auto) {
    auto_changed(changed_control.value_auto);
  }
  control_changed_led->setPixmap(is_expected_value ? green_dot : red_dot);
  control_changed_led->show();
  QTimer::singleShot(5000, this, [this]{ control_changed_led->hide(); });
  emit changed();
}

QString CameraControl::label() const
{
  return tr(qPrintable(control.name));
}



void CameraControl::apply()
{
  set_value(new_value);
}

void CameraControl::restore()
{
  set_value(control);
  control_widget->update(control);
}

void CameraControl::set_value(const Imager::Control &value)
{
  if(control.same_value(value)) {
    new_value = value;
    return;
  }
  qDebug() << "GUI: setting control " << control << " to " << value;
  imager->setControl(value);
  control_widget->update(value);
}

bool CameraControl::is_pending() const
{
  return ! control.same_value(new_value);
}



DPTR_IMPL(CameraControlsWidget)
{
  Imager *imager;
  Configuration::ptr configuration;
  unique_ptr<Ui::CameraControlsWidget> ui;
  unique_ptr<QStringListModel> presetsModel;
  CameraControlsWidget *q;
  list<CameraControl *> control_widgets;
  QMenu *recentPresetsMenu = nullptr;
  QMenu *recentRecordingsMenu = nullptr;
  void controls_changed();
  void loadMenuPreset();
  void saveMenuPreset();
  void pickPresetFromFile();
  void loadPresetFromFile(const QString &path);
  void savePresetToFile();
  void removeSelectedPreset();
  void reloadPresets();
  void selectionChanged();
  bool hasSelection() const;
  QString currentSelection() const;
  void loadToImager(const Configuration::Preset &preset);
  QVariantMap currentPresets() const;
  void reloadRecentlyUsedPresets();
  void reloadRecentRecordings();
  void reloadRecentFilesMenu(QMenu *menu, const QStringList &files);
};



CameraControlsWidget::~CameraControlsWidget()
{
}



CameraControlsWidget::CameraControlsWidget(Imager *imager, const Configuration::ptr &configuration, const FilesystemBrowser::ptr &filesystemBrowser, QWidget *parent)
: QWidget{parent}, dptr(imager, configuration, make_unique<Ui::CameraControlsWidget>(), make_unique<QStringListModel>(), this)
{
  d->ui->setupUi(this);
  d->ui->presetsButton->setMenu(new QMenu);
  d->ui->presets->setModel(d->presetsModel.get());
  d->reloadPresets();
  connect(d->ui->presets, F_PTR(QComboBox, currentIndexChanged, int), this, bind(&Private::selectionChanged, d.get()));
  
  auto addPresetAction = [this](auto action, auto slot, auto signal) {
    d->ui->presetsButton->menu()->addAction(action);
    connect(action, signal, this, slot);
  };
  d->recentPresetsMenu = new QMenu(tr("Recently used presets"), this);
  d->ui->actionShow_only_presets_for_this_camera->setChecked(d->configuration->filter_presets_by_camera());
  addPresetAction(d->ui->actionLoad_selected_preset, bind(&Private::loadMenuPreset, d.get()), &QAction::triggered);
  addPresetAction(d->ui->actionSave_current_settings_as_presets, bind(&Private::saveMenuPreset, d.get()), &QAction::triggered);
  addPresetAction(d->ui->actionRemove_selected_preset, bind(&Private::removeSelectedPreset, d.get()), &QAction::triggered);
  addPresetAction(d->ui->actionLoad_settings_from_file, bind(&Private::pickPresetFromFile, d.get()), &QAction::triggered);
  d->ui->presetsButton->menu()->addMenu(d->recentPresetsMenu);
  if(filesystemBrowser->isLocal()) {
    d->ui->presetsButton->menu()->addMenu(d->recentRecordingsMenu = new QMenu(tr("Recent recordings"), this));
    connect(d->configuration.get(), &Configuration::recording_presets_changed, this, bind(&Private::reloadRecentRecordings, d.get()));
    d->reloadRecentRecordings();
  }
  addPresetAction(d->ui->actionSave_settings_to_file, bind(&Private::savePresetToFile, d.get()), &QAction::triggered);
  addPresetAction(d->ui->actionShow_only_presets_for_this_camera, [this](bool c) {
    d->configuration->set_filter_presets_by_camera(c);
    d->reloadPresets();
    d->reloadRecentlyUsedPresets();
  }, &QAction::toggled);
  
  connect(d->configuration.get(), &Configuration::presets_changed, this, bind(&Private::reloadRecentlyUsedPresets, d.get()));
  d->reloadRecentlyUsedPresets();
  
  auto grid = new QGridLayout(d->ui->controls_box);
  int row = 0;
  for(auto imager_control: imager->controls()) {
    qDebug() << "adding setting: " << imager_control;
    auto control = new CameraControl(imager_control, imager, this);
    d->control_widgets.push_back(control);
    connect(control, &CameraControl::changed, this, bind(&Private::controls_changed, d.get()));
    connect(d->ui->apply, &QPushButton::clicked, control, &CameraControl::apply);
    connect(d->ui->restore, &QPushButton::clicked, control, &CameraControl::restore);
    grid->addWidget(control->controlChangedLed(), row, 0);
    grid->addWidget(new QLabel(control->label()), row, 1);
    grid->addWidget(control->controlWidget(), row, 2);
    grid->addWidget(control->autoValueWidget(), row, 3);
    grid->addWidget(control->onOffValueWidget(), row++, 4);
  }
  connect(d->ui->restore, &QPushButton::clicked, this, bind(&Private::controls_changed, d.get()));
  grid->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Expanding), row, 0, 3);
  grid->setRowStretch(row, 1);
  grid->setColumnStretch(1, 1);
}

void CameraControlsWidget::Private::loadMenuPreset()
{
  auto preset = configuration->load_preset(currentSelection());
  qDebug() << "Importing presets name" << currentSelection() << ":" << preset.name;
  loadToImager(preset);
}

void CameraControlsWidget::Private::saveMenuPreset()
{
  auto name = QInputDialog::getText(q, tr("Save preset as..."), tr("Enter preset name to save current controls") );
  if(name.isEmpty())
    return;
  configuration->add_preset(name, currentPresets());
  reloadPresets();
}

void CameraControlsWidget::Private::removeSelectedPreset()
{
  configuration->remove_preset(currentSelection());
  reloadPresets();
}

void CameraControlsWidget::Private::pickPresetFromFile()
{
  // TODO: last directory used
  auto filename = QFileDialog::getOpenFileName(q, tr("Select Planetary Imager controls file"), configuration->last_controls_folder(), tr("Planetary Imager controls file (*.json)") );
  if(filename.isEmpty())
    return;
  configuration->set_last_controls_folder(QFileInfo{filename}.dir().canonicalPath());
  loadPresetFromFile(filename);
}

void CameraControlsWidget::Private::loadPresetFromFile(const QString& filename)
{  
  loadToImager(Configuration::Preset{filename});
  configuration->preset_saved(filename);
}


void CameraControlsWidget::Private::loadToImager(const Configuration::Preset& preset)
{
  auto controls = preset.load()["controls"].toList();
  for(auto controlWidget: control_widgets)
    controlWidget->importing(controls);
  imager->import_controls(controls);
}


void CameraControlsWidget::Private::savePresetToFile()
{
  // TODO: last directory used
  auto filename = QFileDialog::getSaveFileName(q, tr("Export Planetary Imager controls file"), configuration->last_controls_folder(), tr("Planetary Imager controls file (*.json)") );
  if(filename.isEmpty())
    return;
  configuration->set_last_controls_folder(QFileInfo{filename}.dir().canonicalPath());
  
  QFile file{filename};
  file.open(QIODevice::WriteOnly);
  file.write(QJsonDocument::fromVariant(currentPresets()).toJson());
  configuration->preset_saved(filename);
}

QVariantMap CameraControlsWidget::Private::currentPresets() const
{
  return {{"camera", imager->name()}, {"controls", imager->export_controls()}};
}


void CameraControlsWidget::Private::reloadPresets()
{
  auto presets = configuration->list_presets();
  if(configuration->filter_presets_by_camera()) {
    presets.erase(remove_if(begin(presets), end(presets), [this](const auto &preset) { return not preset.isFor(imager->name()); }), presets.end());
  }
  QStringList names;
  for(const auto &preset: presets)
    names << preset.name;
  presetsModel->setStringList(names);
  selectionChanged();
}

void CameraControlsWidget::Private::selectionChanged()
{
  ui->actionLoad_selected_preset->setEnabled(hasSelection());
  ui->actionRemove_selected_preset->setEnabled(hasSelection());
}

bool CameraControlsWidget::Private::hasSelection() const
{
  return ui->presets->currentIndex() != -1;
}

QString CameraControlsWidget::Private::currentSelection() const
{
  if(! hasSelection() )
    return {};
  return presetsModel->stringList()[ui->presets->currentIndex()];
}


void CameraControlsWidget::Private::controls_changed()
{
  bool any_changed = std::any_of(control_widgets.begin(), control_widgets.end(), bind(&CameraControl::is_pending, _1));
  ui->apply->setEnabled(any_changed);
  ui->restore->setEnabled(any_changed);
}

void CameraControlsWidget::Private::reloadRecentlyUsedPresets()
{
  reloadRecentFilesMenu(recentPresetsMenu, configuration->latest_exported_presets());
}

void CameraControlsWidget::Private::reloadRecentRecordings()
{
  reloadRecentFilesMenu(recentRecordingsMenu, configuration->latest_recording_presets());
}


void CameraControlsWidget::Private::reloadRecentFilesMenu(QMenu* menu, const QStringList &files)
{
  if(!menu)
    return;
  menu->clear();
  size_t items_loaded = 0;
  for(auto recentFile: files) {
    if(configuration->filter_presets_by_camera() && not Configuration::Preset{recentFile}.isFor(imager->name()))
      continue;
      connect(menu->addAction(recentFile), &QAction::triggered, q, bind(&Private::loadPresetFromFile, this, recentFile));
      ++items_loaded;
  }
  menu->setEnabled(items_loaded > 0);
}


#include "cameracontrolswidget.moc"
