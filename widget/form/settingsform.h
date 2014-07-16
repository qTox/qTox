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

#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QObject>
#include <QSpacerItem>
#include <QCheckBox>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include "widget/tool/clickablelabel.h"
#include "ui_widget.h"
#include "widget/selfcamview.h"

class SettingsForm : public QObject
{
    Q_OBJECT
public:
    SettingsForm();
    ~SettingsForm();

    void show(Ui::Widget& ui);

public slots:
    void setFriendAddress(const QString& friendAddress);

private slots:
    void onTestVideoClicked();
    void onEnableIPv6Updated();
    void onUseTranslationUpdated();
    void onMakeToxPortableUpdated();
    void copyIdClicked();

private:
    QLabel headLabel;/*, nameLabel, statusTextLabel;*/
    QTextEdit id;
    ClickableLabel idLabel;
    QLabel profilesLabel;
    QComboBox profiles;
    QPushButton loadConf, exportConf, delConf, importConf, videoTest;
    QHBoxLayout cbox, buttons;
    QCheckBox enableIPv6, useTranslations, makeToxPortable;
    QVBoxLayout layout, headLayout;
    QWidget *main, *head, *hboxcont1, *hboxcont2;
    void populateProfiles();

public:
    //QLineEdit name, statusText;
};

#endif // SETTINGSFORM_H
