/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include <QWidget>

class ContentLayout;
class GroupInvite;
class GroupInviteWidget;

class QGroupBox;
class QLabel;
class QPushButton;
class QScrollArea;
class QSignalMapper;
class Settings;
class Core;

namespace Ui {
class MainWindow;
}

class GroupInviteForm : public QWidget
{
    Q_OBJECT
public:
    GroupInviteForm(Settings& settings, Core& core);
    ~GroupInviteForm();

    void show(ContentLayout* contentLayout);
    bool addGroupInvite(const GroupInvite& inviteInfo);
    bool isShown() const;

signals:
    void groupCreate(uint8_t type);
    void groupInviteAccepted(const GroupInvite& inviteInfo);
    void groupInvitesSeen();

protected:
    void showEvent(QShowEvent* event) final;

private:
    void retranslateUi();
    void deleteInviteWidget(const GroupInvite& inviteInfo);

private:
    QWidget* headWidget;
    QLabel* headLabel;
    QPushButton* createButton;
    QGroupBox* inviteBox;
    QList<GroupInviteWidget*> invites;
    QScrollArea* scroll;
    Settings& settings;
    Core& core;
};
