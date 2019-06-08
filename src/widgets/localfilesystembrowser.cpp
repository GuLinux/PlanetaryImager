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

#include "localfilesystembrowser.h"
#include <QFileDialog>

using namespace std;
DPTR_IMPL(LocalFilesystemBrowser) {
};

LocalFilesystemBrowser::LocalFilesystemBrowser(QObject* parent) : dptr()
{
}

LocalFilesystemBrowser::~LocalFilesystemBrowser()
{
}


void LocalFilesystemBrowser::pickDirectory(const QString currentDirectory) const
{
    QFileDialog *filedialog = new QFileDialog();
    filedialog->setFileMode(QFileDialog::Directory);
    filedialog->setDirectory(currentDirectory);
    filedialog->setOption(QFileDialog::ShowDirsOnly);
    connect(filedialog, &QFileDialog::fileSelected, this, &FilesystemBrowser::directoryPicked);
    connect(filedialog, SIGNAL(finished(int)), filedialog, SLOT(deleteLater()));
    filedialog->show();
}
