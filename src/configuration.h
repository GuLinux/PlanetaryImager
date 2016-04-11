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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include "dptr.h"
#include <QString>

class QSettings;
class Configuration
{
public:
    Configuration(QSettings &settings);
    ~Configuration();
        
    long long maxMemoryUsage() const;
    void setMaxMemoryUsage(long long memoryUsage);
    
    bool bufferedOutput() const;
    void setBufferedOutput(bool buffered);
    
    int maxPreviewFPSOnSaving() const;
    void setMaxPreviewFPSOnSaving(int maxFPS);
    
    void setRecordingFramesLimit(long long limit);
    long long recordingFramesLimit() const;
    
    void setSaveDirectory(const QString &directory);
    QString saveDirectory() const;
    
    void setSaveFilePrefix(const QString &prefix);
    QString saveFilePrefix() const;
    
    void setSaveFileSuffix(const QString &suffix);
    QString saveFileSuffix() const;
    
    void setObserver(const QString &observer);
    QString observer() const;
    
    void setTelescope(const QString &telescope);
    QString telescope() const;
    
    QString savefile() const;
    
    enum EdgeAlgorithm { Sobel, Canny, SobelDeprecated };
    
    void setEdgeAlgorithm(EdgeAlgorithm algorithm);
    EdgeAlgorithm edgeAlgorithm() const;
    
    void setSobelKernel(int size);
    int sobelKernel() const;
    
    void setSobelBlurSize(int size);
    int sobelBlurSize() const;
    
    void setSobelScale(double scale);
    double sobelScale() const;
    
    void setSobelDelta(double delta);
    double sobelDelta() const;
    
    double cannyLowThreshold() const;
    void setCannyLowThreshold(double threshold);
    
    double cannyThresholdRatio() const;
    void setCannyThresholdRatio(double ratio);
    
    int cannyKernelSize() const;
    void setCannyKernelSize(int size);
    
    int cannyBlurSize() const;
    void setCannyBlurSize(int size);
    
    void resetCannyAdvancedSettings();
    void resetSobelAdvancedSettings();

    enum SaveFormat { SER, Video };
    void setSaveFormat(SaveFormat format);
    SaveFormat saveFormat() const;
    
    QString videoCodec() const;
    void setVideoCodec(const QString &codec);
    
    bool save_info_file() const;
    void set_save_info_file(bool save);
private:
  D_PTR
};


#endif // CONFIGURATION_H
