/*
    Copyright © 2014-2015 by The qTox Project

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

#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QHBoxLayout>
#include <QPushButton>
#include <QStyleFactory>
#include <array>

class Camera;
class GenericForm;
class GeneralForm;
class PrivacyForm;
class AVForm;
class QLabel;
class QTabWidget;
class ContentLayout;

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget* parent = nullptr);
    ~SettingsWidget();

    bool isShown() const;
    void show(ContentLayout* contentLayout);
    void setBodyHeadStyle(QString style);

    void showAbout();

private slots:
    void onTabChanged(int);

private:
    void retranslateUi();

private:
    QWidget *head, *body;
    QTabWidget *settingsWidgets;
    QLabel *nameLabel, *imgLabel;
    std::array<GenericForm*, 6> cfgForms;
    int currentIndex;
};

#endif // SETTINGSWIDGET_H
