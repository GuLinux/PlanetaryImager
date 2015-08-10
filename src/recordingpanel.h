/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#ifndef RECORDINGPANEL_H
#define RECORDINGPANEL_H

#include <QWidget>
#include "dptr.h"

class Configuration;
class RecordingPanel : public QWidget
{
    Q_OBJECT
public:
    ~RecordingPanel();
    RecordingPanel(Configuration &configuration, QWidget* parent = 0);
public slots:
  void recording(bool recording = false, const QString &filename = {});
  void saveFPS(double fps);
  void saved(int frames);
  void dropped(int frames);
signals:
  void start();
  void stop();
private:
  D_PTR;
};

#endif // RECORDINGPANEL_H
