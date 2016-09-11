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

#include "savefileconfiguration.h"
#include "ui_savefileconfiguration.h"
#include "configuration.h"
#include <QStringListModel>
#include <QDebug>

using namespace std;
using namespace std::placeholders;

DPTR_IMPL(SaveFileConfiguration) {
    SaveFileConfiguration *q;
    unique_ptr<Ui::SaveFileConfiguration> ui;
};

SaveFileConfiguration::SaveFileConfiguration(Configuration& configuration, QWidget* parent) : dptr(this)
{
    d->ui.reset(new Ui::SaveFileConfiguration);
    d->ui->setupUi(this);
    d->ui->separator->setText(configuration.save_file_prefix_suffix_separator());
    connect(d->ui->separator, &QLineEdit::textChanged, bind(&Configuration::set_save_file_prefix_suffix_separator, &configuration, _1));
    d->ui->prefixes->setText(configuration.save_file_avail_prefixes().join(","));
    d->ui->suffixes->setText(configuration.save_file_avail_suffixes().join(","));
    auto split = [](const QString &s) {
        QStringList splitted = s.split(",");
        for_each(splitted.begin(), splitted.end(), [](QString &s) { s = s.trimmed(); });
        splitted.removeDuplicates();
        splitted.erase(remove_if(splitted.begin(), splitted.end(), bind(&QString::isEmpty, _1)), splitted.end());
        return splitted;
    };
    connect(d->ui->prefixes, &QLineEdit::textChanged, [&, split](const QString &v) { configuration.set_save_file_avail_prefixes(split(v)); });
    connect(d->ui->suffixes, &QLineEdit::textChanged, [&, split](const QString &v) { configuration.set_save_file_avail_suffixes(split(v)); });
}

SaveFileConfiguration::~SaveFileConfiguration()
{
}
