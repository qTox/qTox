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

#include <QHBoxLayout>
#include <QPushButton>
class GenericForm;
class GeneralForm;
class IdentityForm;
class PrivacyForm;
class AVForm;
namespace Ui {class MainWindow;};

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    SettingsWidget();
    ~SettingsWidget();

    void show(Ui::MainWindow &ui);
    
    QWidget *head, *body; // keep the others private

public slots:
    //void setFriendAddress(const QString& friendAddress);

private slots:
    void onGeneralClicked();
    void onIdentityClicked();
    void onPrivacyClicked();
    void onAVClicked();

private:
    QWidget *main, *foot;
    // main consists of body+foot for Ui::MainWindow
    QVBoxLayout *mainLayout;
    
    GenericForm* active;
    GeneralForm* generalForm;
    IdentityForm* identityForm;
    PrivacyForm* privacyForm;
    AVForm* avForm;
    void hideSettingsForms();
    
    
    // the code pertaining to the icons is mostly copied from ui_mainwindow.h
    QHBoxLayout *iconsLayout;
    QPushButton *generalButton;
    QPushButton *identityButton;
    QPushButton *privacyButton;
    QPushButton *avButton;
    void prepButtons(); // just so I can move the crap to the bottom of the file
};

#endif // SETTINGSWIDGET_H
