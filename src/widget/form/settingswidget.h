/*
    Copyright © 2014-2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QHBoxLayout>
#include <QPushButton>
#include <QStyleFactory>

#include <array>
#include <memory>

class Camera;
class Core;
class GenericForm;
class GeneralForm;
class IAudioControl;
class PrivacyForm;
class AVForm;
class QLabel;
class QTabWidget;
class ContentLayout;
class UpdateCheck;
class Widget;

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    SettingsWidget(UpdateCheck* updateCheck, IAudioControl& audio, Core *core, Widget* parent = nullptr);
    ~SettingsWidget();

    bool isShown() const;
    void show(ContentLayout* contentLayout);
    void setBodyHeadStyle(QString style);

    void showAbout();

public slots:
    void onUpdateAvailable(void);

private slots:
    void onTabChanged(int);

private:
    void retranslateUi();

private:
    std::unique_ptr<QVBoxLayout> bodyLayout;
    std::unique_ptr<QTabWidget> settingsWidgets;
    std::array<std::unique_ptr<GenericForm>, 6> cfgForms;
    int currentIndex;
};
