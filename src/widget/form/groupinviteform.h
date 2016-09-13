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
#include <QDateTime>
#include <QSet>
#include <QScrollArea>

#include "src/widget/contentwidget.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/gui.h"

class QLabel;
class QVBoxLayout;
class QPushButton;
class QGroupBox;
class QSignalMapper;

namespace Ui {class MainWindow;}

class GroupInviteForm : public ContentWidget
{
    Q_OBJECT
public:
    explicit GroupInviteForm(QWidget* parent = nullptr);
    ~GroupInviteForm();

    void addGroupInvite(uint32_t friendId, uint8_t type, QByteArray invite);
    bool isShown() const;

signals:
    void groupCreate(uint8_t type);
    void groupInviteAccepted(uint32_t friendId, uint8_t type, QByteArray invite);
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
    void retranslateGroupLabel(CroppingLabel* label);
    void deleteInviteButtons(QWidget* widget);

private:
    struct GroupInvite
    {
        uint32_t friendId;
        uint8_t type;
        QByteArray invite;
        QDateTime time;
    };

    QWidget* headWidget;
    QWidget* bodyWidget;
    QLabel* headLabel;
    QPushButton* createButton;
    QGroupBox* inviteBox;
    QVBoxLayout* inviteLayout;
    QSet<QPushButton*> acceptButtons;
    QSet<QPushButton*> rejectButtons;
    QSet<CroppingLabel*> groupLabels;
    QList<GroupInvite> groupInvites;
    QScrollArea* scroll;
};

#endif // GROUPINVITEFORM_H
