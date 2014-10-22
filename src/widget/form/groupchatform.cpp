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

#include "groupchatform.h"
#include "tabcompleter.h"
#include "src/group.h"
#include "src/widget/groupwidget.h"
#include "src/widget/tool/chattextedit.h"
#include "src/widget/croppinglabel.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/core.h"
#include "src/misc/style.h"
#include <QPushButton>
#include <QMimeData>
#include <QDragEnterEvent>
#include "src/historykeeper.h"

GroupChatForm::GroupChatForm(Group* chatGroup)
    : group(chatGroup)
{
    nusersLabel = new QLabel();
    namesList = new QLabel();
    namesList->setObjectName("peersLabel");

    tabber = new TabCompleter(msgEdit, group);

    fileButton->setEnabled(false);
    callButton->setVisible(false);
    videoButton->setVisible(false);
    volButton->setVisible(false);
    micButton->setVisible(false);

    nameLabel->setText(group->widget->getName());

    nusersLabel->setFont(Style::getFont(Style::Medium));
    nusersLabel->setText(GroupChatForm::tr("%1 users in chat","Number of users in chat").arg(group->peers.size()));
    nusersLabel->setObjectName("statusLabel");

    avatar->setPixmap(QPixmap(":/img/group_dark.png"), Qt::transparent);

    namesList->setText(QStringList(group->peers.values()).join(", "));

    msgEdit->setObjectName("group");

    headTextLayout->addWidget(nusersLabel);
    headTextLayout->addWidget(namesList);
    headTextLayout->setMargin(0);
    headTextLayout->setSpacing(0);
    headTextLayout->addStretch();

    connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(msgEdit, SIGNAL(enterPressed()), this, SLOT(onSendTriggered()));
    connect(msgEdit, &ChatTextEdit::tabPressed, tabber, &TabCompleter::complete);
    connect(msgEdit, &ChatTextEdit::keyPressed, tabber, &TabCompleter::reset);

    setAcceptDrops(true);
}

void GroupChatForm::onSendTriggered()
{
    QString msg = msgEdit->toPlainText();
    if (msg.isEmpty())
        return;
    msgEdit->clear();
    emit sendMessage(group->groupId, msg);
}

void GroupChatForm::onUserListChanged()
{
    nusersLabel->setText(tr("%1 users in chat").arg(group->nPeers));
    namesList->setText(QStringList(group->peers.values()).join(", "));
}

void GroupChatForm::dragEnterEvent(QDragEnterEvent *ev)
{
    if (ev->mimeData()->hasFormat("friend"))
        ev->acceptProposedAction();
}

void GroupChatForm::dropEvent(QDropEvent *ev)
{
    if (ev->mimeData()->hasFormat("friend"))
    {
        int friendId = ev->mimeData()->data("friend").toInt();
        Core::getInstance()->groupInviteFriend(friendId, group->groupId);
    }
}

