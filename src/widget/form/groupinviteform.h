/*
    Copyright © 2015 by The qTox Project

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

#ifndef GROUPINVITEFORM_H
#define GROUPINVITEFORM_H

#include <QWidget>
#include <QSet>
#include "src/widget/gui.h"

class QLabel;
class QVBoxLayout;
class QPushButton;
class QGroupBox;
class QSignalMapper;

class ContentLayout;

namespace Ui {class MainWindow;}

class GroupInviteForm : public QWidget
{
    Q_OBJECT
public:
    GroupInviteForm();
    ~GroupInviteForm();

    void show(ContentLayout *contentLayout);
    void addGroupInvite(int32_t friendId, uint8_t type, QByteArray invite);
    bool isShown() const;

signals:
    void groupCreate(uint8_t type);
    void groupInviteAccepted(int32_t friendId, uint8_t type, QByteArray invite);
    void groupInvitesSeen();

protected:
    void showEvent(QShowEvent* event) final override;

private slots:
    void onGroupInviteAccepted();
    void onGroupInviteRejected();

private:
    void retranslateUi();
    void retranslateAcceptButton(QPushButton* acceptButton);
    void retranslateRejectButton(QPushButton* rejectButton);

private:
    struct GroupInvite
    {
        int32_t friendId;
        uint8_t type;
        QByteArray invite;
    };

    QWidget* headWidget;
    QLabel* headLabel;
    QPushButton* createButton;
    QGroupBox* inviteBox;
    QVBoxLayout* inviteLayout;
    QSet<QPushButton*> acceptButtons;
    QSet<QPushButton*> rejectButtons;
    QList<GroupInvite> groupInvites;
};

#endif // GROUPINVITEFORM_H
