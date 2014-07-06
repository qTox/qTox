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

#ifndef SELFCAMVIEW_H
#define SELFCAMVIEW_H

#include <QWidget>
#include <QHBoxLayout>
#include <QTimer>
#include <QLabel>
#include "camera.h"

class QCloseEvent;
class QShowEvent;
class QPainter;

class SelfCamView : public QWidget
{
    Q_OBJECT

public:
    SelfCamView(Camera* Cam, QWidget *parent=0);
    ~SelfCamView();

private slots:
    void updateDisplay();

private:
    void closeEvent(QCloseEvent*);
    void showEvent(QShowEvent*);
    void paint(QPainter *painter);

private:
    QLabel *displayLabel;
    QHBoxLayout* mainLayout;
    Camera* cam;
    QTimer updateDisplayTimer;
};

#endif // SELFCAMVIEW_H
