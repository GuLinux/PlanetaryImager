/*
 * Copyright (C) 2019  Marco Gulino <marco@gulinux.net>
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


#include "mainwindowwidgets.h"
#include "commons/configuration.h"
#include <QMainWindow>
#include <QList>
#include <QDockWidget>
#include <QAction>
#include <QMenu>
#include <QDebug>
#include <QVariantMap>
#include <QVariantList>
#include <QJsonDocument>
#include <QToolBar>
#include <algorithm>

using namespace std;

DPTR_IMPL(MainWindowWidgets) {
    QMainWindow *main_window;
    QMenu *window_menu;
    Configuration &configuration;
    bool is_first_run;

    QAction *docks_separator;
    QAction *toolbars_separator;
    QList<QDockWidget*> docks;
    QList<QToolBar*> toolbars;
};

MainWindowWidgets::MainWindowWidgets(QMainWindow *main_window, QMenu *windowMenu, Configuration &configuration) : dptr(main_window, windowMenu, configuration, !configuration.widgets_setup_first_run()) {
    configuration.set_widgets_setup_first_run(true);
    windowMenu->addSection(QObject::tr("Panels"));
    d->docks_separator = windowMenu->addSeparator();
    windowMenu->addAction(QObject::tr("Show All"), main_window, [this]{
        for(auto dock: d->docks) {
            dock->show();
        }
    });
    windowMenu->addAction(QObject::tr("Hide All"), main_window, [this] {
        for(auto dock: d->docks) {
            dock->hide();
        }
    });

    windowMenu->addSection(QObject::tr("Toolbars"));
    d->toolbars_separator = windowMenu->addSeparator();
    windowMenu->addAction(QObject::tr("Show All"), main_window, [this]{
        for(auto toolbar: d->toolbars) {
            toolbar->show();
        }
    });
    windowMenu->addAction(QObject::tr("Hide All"), main_window, [this] {
        for(auto toolbar: d->toolbars) {
            toolbar->hide();
        }
    });

}

MainWindowWidgets::~MainWindowWidgets() {
}


void MainWindowWidgets::save() {
    d->configuration.set_dock_status(d->main_window->saveState());
}

void MainWindowWidgets::load() {
    d->main_window->restoreState(d->configuration.dock_status());
    for(auto dock: d->docks) {
        d->main_window->restoreDockWidget(dock);
    }
}

void MainWindowWidgets::add_dock(QDockWidget *widget) {
    if(d->is_first_run && !d->docks.isEmpty()) {
        d->main_window->tabifyDockWidget(d->docks.first(), widget);
    }

    d->window_menu->insertAction(d->docks_separator, widget->toggleViewAction());
    d->docks.push_back({widget});
}

void MainWindowWidgets::add_toolbar(QToolBar *toolbar, bool add_to_main_window) {
    d->toolbars.push_back(toolbar);
    if(add_to_main_window) {
        d->main_window->addToolBar(toolbar);
    }
    d->window_menu->insertAction(d->toolbars_separator, toolbar->toggleViewAction());
}
