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

#include "pickdirectory.h"
#include "network/client/remotefilesystem.h"
#include "ui_pickdirectory.h"
#include <QStandardItem>
#include <QStandardItemModel>
#include <QIcon>

using namespace std;


DPTR_IMPL(PickDirectory) {
  unique_ptr<Ui::PickDirectory> ui;
  shared_ptr<QStandardItemModel> model;
  RemoteFilesystemPtr filesystem;
  PickDirectory *q;
  void browse(const QString &path);
  static const int PathRole;
  QString currentPath;
};

const int PickDirectory::Private::PathRole = Qt::UserRole +1;

PickDirectory::PickDirectory(const RemoteFilesystemPtr& filesystem, const QString& startingDirectory)
  : dptr(make_unique<Ui::PickDirectory>(), make_shared<QStandardItemModel>(), filesystem, this)
{
  d->ui->setupUi(this);
  d->ui->filesystem->setModel(d->model.get());
  d->browse(startingDirectory);
  connect(d->ui->filesystem, &QListView::doubleClicked, [=](const QModelIndex &index) {
    d->browse(d->model->itemData(index)[Private::PathRole].toString());
  });
  connect(this, &QDialog::accepted, this, [=]{ emit directoryPicked(d->currentPath); });
}

void PickDirectory::Private::browse(const QString& path)
{
  currentPath = path;
  ui->path->setText(path);
  model->clear();
  auto entry = filesystem->entry(path);
  auto entries = entry->children();
  for(auto entry: entries) {
    if(entry->type() == FilesystemEntry::Directory) {
      auto item = new QStandardItem{QIcon(":/resources/folder.png"), entry->name()};
      item->setData(entry->path(), PathRole);
      model->appendRow(item);
    }
  }
}


PickDirectory::~PickDirectory()
{
}

#include "pickdirectory.moc"
