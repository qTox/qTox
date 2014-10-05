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
#include "widget/selfcamview.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
class Camera;

class AVForm : public GenericForm
{
    Q_OBJECT
public:
    AVForm(Camera* cam);
    ~AVForm();

private slots:
    void onTestVideoPressed();
private:
    QGroupBox* videoGroup;
    QVBoxLayout* videoLayout;
    QPushButton* testVideo;
    SelfCamView* camView;
    
    void showTestVideo();
    void closeTestVideo();
       
};

#endif
