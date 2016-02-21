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

#include "groupinviteform.h"

#include <tox/tox.h>
#include <QSignalMapper>
#include <QPushButton>
#include <QBoxLayout>
#include <QGroupBox>
#include <QDateTime>
#include <QLabel>
#include "ui_mainwindow.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/translator.h"
#include "src/nexus.h"
#include "src/core/core.h"
#include "src/widget/gui.h"
#include "src/widget/translator.h"
#include "src/widget/contentlayout.h"

GroupInviteForm::GroupInviteForm()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    createButton = new QPushButton(this);
    connect(createButton, &QPushButton::released, [this]()
    {
        emit groupCreate(TOX_GROUPCHAT_TYPE_AV);
    });

    inviteBox = new QGroupBox(this);
    inviteLayout = new QVBoxLayout(inviteBox);
    inviteLayout->addStretch(1);

    layout->addWidget(createButton);
    layout->addWidget(inviteBox);

    QFont bold;
    bold.setBold(true);

    headLabel = new QLabel(this);
    headLabel->setFont(bold);
    headWidget = new QWidget(this);
    QHBoxLayout* headLayout = new QHBoxLayout(headWidget);
    headLayout->addWidget(headLabel);

    retranslateUi();
    Translator::registerHandler(std::bind(&GroupInviteForm::retranslateUi, this), this);
}

void GroupInviteForm::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(this);
    contentLayout->mainHead->layout()->addWidget(headWidget);
    QWidget::show();
    headWidget->show();
}

void GroupInviteForm::addGroupInvite(int32_t friendId, uint8_t type, QByteArray invite)
{
    QWidget* groupWidget = new QWidget(this);
    QHBoxLayout* groupLayout = new QHBoxLayout(groupWidget);

    CroppingLabel* groupLabel = new CroppingLabel(this);
    groupLabel->setText(tr("Invited by <b>%1</b> on %2.").arg(Nexus::getCore()->getFriendUsername(friendId), QDateTime::currentDateTime().toString()));
    groupLayout->addWidget(groupLabel);

    QPushButton* acceptButton = new QPushButton(this);
    acceptButtons.insert(acceptButton);
    connect(acceptButton, &QPushButton::clicked, this, &GroupInviteForm::onGroupInviteAccepted);
    groupLayout->addWidget(acceptButton);
    retranslateAcceptButton(acceptButton);

    QPushButton* rejectButton = new QPushButton(this);
    rejectButtons.insert(rejectButton);
    connect(rejectButton, &QPushButton::clicked, this, &GroupInviteForm::onGroupInviteRejected);
    groupLayout->addWidget(rejectButton);
    retranslateRejectButton(rejectButton);

    inviteLayout->insertWidget(0, groupWidget);

    GroupInvite group;
    group.friendId = friendId;
    group.type = type;
    group.invite = invite;
    groupInvites.push_front(group);

    if (isVisible())
        emit groupInvitesSeen();
}

void GroupInviteForm::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    emit groupInvitesSeen();
}

void GroupInviteForm::onGroupInviteAccepted()
{
    QPushButton* acceptButton = static_cast<QPushButton*>(sender());
    QWidget* groupWidget = acceptButton->parentWidget();
    int index = inviteLayout->indexOf(groupWidget);
    GroupInvite invite = groupInvites.at(index);
    groupInvites.removeAt(index);

    groupWidget->deleteLater();
    inviteLayout->removeWidget(groupWidget);

    emit groupInviteAccepted(invite.friendId, invite.type, invite.invite);
}

void GroupInviteForm::onGroupInviteRejected()
{
    QPushButton* rejectButton = static_cast<QPushButton*>(sender());
    QWidget* groupWidget = rejectButton->parentWidget();
    int index = inviteLayout->indexOf(groupWidget);
    groupInvites.removeAt(index);

    groupWidget->deleteLater();
    inviteLayout->removeWidget(groupWidget);
}

void GroupInviteForm::retranslateUi()
{
    headLabel->setText(tr("Groups"));
    createButton->setText(tr("Create new group"));
    inviteBox->setTitle(tr("Group invites"));

    for (QPushButton* acceptButton : acceptButtons)
        retranslateAcceptButton(acceptButton);

    for (QPushButton* rejectButton : rejectButtons)
        retranslateRejectButton(rejectButton);
}

void GroupInviteForm::retranslateAcceptButton(QPushButton *acceptButton)
{
    acceptButton->setText(tr("Join"));
}

void GroupInviteForm::retranslateRejectButton(QPushButton *rejectButton)
{
    rejectButton->setText(tr("Decline"));
}
