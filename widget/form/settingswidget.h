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

#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>

#include "widget/croppinglabel.h"

namespace Ui {class MainWindow;}
class QString;

class SettingsWidget : public QObject
{
    Q_OBJECT
public:
    SettingsWidget();
    ~SettingsWidget();

    void show(Ui::MainWindow &ui);

public slots:
    //void setFriendAddress(const QString& friendAddress);

private slots:

private:
    QWidget *main, *head;
    // the code pertaining to the icons is mostly copied from ui_mainwindow.h
    QHBoxLayout *iconsLayout;
    QPushButton *generalButton;
    QPushButton *identityButton;
    QPushButton *privacyButton;
    QPushButton *avButton;
    
    // now the actual pages and stuff
    // ...
public:
};

#endif // SETTINGSFORM_H
