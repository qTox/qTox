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
#include "group.h"
#include "widget/groupwidget.h"
#include "widget/widget.h"
#include "friend.h"
#include "friendlist.h"
#include "style.h"
#include <QFont>
#include <QTime>
#include <QScrollBar>
#include <QMenu>
#include <QFile>
#include <QFileDialog>

GroupChatForm::GroupChatForm(Group* chatGroup)
    : group(chatGroup)
{
    nusers = new QLabel();
    namesList = new QLabel();

    fileButton->setEnabled(false);
    callButton->setVisible(false);
    videoButton->setVisible(false);
    volButton->setVisible(false);
    micButton->setVisible(false);

    QFont small;
    small.setPixelSize(10);

    nameLabel->setText(group->widget->name.text());
    nusers->setFont(small);
    nusers->setText(GroupChatForm::tr("%1 users in chat","Number of users in chat").arg(group->peers.size()));
    avatarLabel->setPixmap(QPixmap(":/img/group_dark.png"));

    QString names;
    for (QString& s : group->peers)
        names.append(s+", ");
    names.chop(2);
    namesList->setText(names);
    namesList->setFont(small);

    msgEdit->setObjectName("group");

    mainChatLayout->setColumnStretch(1,1);
    mainChatLayout->setHorizontalSpacing(10);

    headTextLayout->addWidget(nusers);
    headTextLayout->addWidget(namesList);
    headTextLayout->setMargin(0);
    headTextLayout->setSpacing(0);
    headTextLayout->addStretch();

    connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(msgEdit, SIGNAL(enterPressed()), this, SLOT(onSendTriggered()));
    connect(chatArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(onSliderRangeChanged()));
    connect(chatArea, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
}

GroupChatForm::~GroupChatForm()
{

}

void GroupChatForm::onSendTriggered()
{
    QString msg = msgEdit->toPlainText();
    if (msg.isEmpty())
        return;
    msgEdit->clear();
    emit sendMessage(group->groupId, msg);
}

void GroupChatForm::addGroupMessage(QString message, int peerId)
{
    QString msgAuthor;
    if (group->peers.contains(peerId))
        msgAuthor = group->peers[peerId];
    else
        msgAuthor = tr("<Unknown>");

    addMessage(msgAuthor, message);
}

void GroupChatForm::onUserListChanged()
{
    nusers->setText(tr("%1 users in chat").arg(group->nPeers));
    QString names;
    for (QString& s : group->peers)
        names.append(s+", ");
    names.chop(2);
    namesList->setText(names);
}
