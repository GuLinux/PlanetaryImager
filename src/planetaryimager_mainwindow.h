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

#ifndef QHYMAINWINDOW_H
#define QHYMAINWINDOW_H

#include <QMainWindow>
#include "dptr.h"
#include "commons/messageslogger.h"
#include "commons/filesystembrowser.h"
#include "image_handlers/imagehandler.h"
#include "planetaryimager.h"
namespace Ui
{
class PlanetaryImagerMainWindow;
}

class PlanetaryImagerMainWindow : public QMainWindow
{
  Q_OBJECT
public:
  ~PlanetaryImagerMainWindow();
  PlanetaryImagerMainWindow(
      const PlanetaryImager::ptr &planetaryImager,
      const ImageHandlers::ptr &imageHandlers,
      const FilesystemBrowser::ptr &filesystemBrowser,
      const QString &logFilePath = {},
      QWidget* parent = 0,
      Qt::WindowFlags flags = 0
  );
  void connectCamera(const Driver::Camera::ptr &camera);
  ImageHandler::ptr imageHandler() const;
  Imager *imager() const;
public slots:
  void setImager(Imager *imager);
  void notify(const QDateTime &when, MessagesLogger::Type notification_type, const QString &title, const QString &message);

private slots:
    void updateInfoOverlay(); ///< Updates positions of block matching targets for display

protected:
  void closeEvent(QCloseEvent *event) override;
signals:
  void quit();
  void imagerChanged();
private:
  DPTR
};

#endif // QHYMAINWINDOW_H
