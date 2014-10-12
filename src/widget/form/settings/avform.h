/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef AVFORM_H
#define AVFORM_H

#include "genericsettings.h"
#include "src/widget/videosurface.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>

namespace Ui {
class AVSettings;
}

class Camera;

class AVForm : public GenericForm
{
    Q_OBJECT
public:
    AVForm();
    ~AVForm();
    virtual void present();

private slots:

    void on_ContrastSlider_sliderMoved(int position);
    void on_SaturationSlider_sliderMoved(int position);
    void on_BrightnessSlider_sliderMoved(int position);
    void on_HueSlider_sliderMoved(int position);
    void on_videoModescomboBox_currentIndexChanged(const QString &arg1);

private:
    Ui::AVSettings *bodyUI;
    VideoSurface* camView;
};

#endif
