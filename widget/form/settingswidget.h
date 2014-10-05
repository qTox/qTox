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
class Camera;
class GenericForm;
class GeneralForm;
class IdentityForm;
class PrivacyForm;
class AVForm;
class QTabWidget;

namespace Ui {class MainWindow;}

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    SettingsWidget(Camera* cam, QWidget* parent = nullptr);
    ~SettingsWidget();

    void show(Ui::MainWindow &ui);
    IdentityForm *getIdentityForm() {return ifrm;}

private slots:
    void onTabChanged(int);

private:
    QWidget *head, *body; // keep the others private
    IdentityForm *ifrm;
    QTabWidget *settingsTabs;
};

#endif // SETTINGSWIDGET_H
