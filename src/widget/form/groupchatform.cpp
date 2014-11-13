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
#include "src/misc/flowlayout.h"
#include <QDebug>

GroupChatForm::GroupChatForm(Group* chatGroup)
    : group(chatGroup), inCall{false}
{
    nusersLabel = new QLabel();

    tabber = new TabCompleter(msgEdit, group);

    fileButton->setEnabled(false);
    if (group->avGroupchat)
    {
        videoButton->setEnabled(false);
        videoButton->setObjectName("grey");
    }
    else
    {
        videoButton->setVisible(false);
        callButton->setVisible(false);
        volButton->setVisible(false);
        micButton->setVisible(false);
    }

    nameLabel->setText(group->widget->getName());

    nusersLabel->setFont(Style::getFont(Style::Medium));
    nusersLabel->setText(GroupChatForm::tr("%1 users in chat","Number of users in chat").arg(group->peers.size()));
    nusersLabel->setObjectName("statusLabel");

    avatar->setPixmap(QPixmap(":/img/group_dark.png"), Qt::transparent);

    msgEdit->setObjectName("group");

    namesListLayout = new FlowLayout(0,5,0);
    QStringList names(group->peers.values());
    for (const QString& name : names)
        namesListLayout->addWidget(new QLabel(name));

    headTextLayout->addWidget(nusersLabel);
    headTextLayout->addLayout(namesListLayout);
    headTextLayout->addStretch();

    nameLabel->setMinimumHeight(12);
    nusersLabel->setMinimumHeight(12);

    connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(msgEdit, SIGNAL(enterPressed()), this, SLOT(onSendTriggered()));
    connect(msgEdit, &ChatTextEdit::tabPressed, tabber, &TabCompleter::complete);
    connect(msgEdit, &ChatTextEdit::keyPressed, tabber, &TabCompleter::reset);
    connect(callButton, &QPushButton::clicked, this, &GroupChatForm::onCallClicked);
    connect(micButton, SIGNAL(clicked()), this, SLOT(onMicMuteToggle()));
    connect(volButton, SIGNAL(clicked()), this, SLOT(onVolMuteToggle()));

    setAcceptDrops(true);
}

void GroupChatForm::onSendTriggered()
{
    QString msg = msgEdit->toPlainText();
    if (msg.isEmpty())
        return;

    msgEdit->clear();

    if (msg.startsWith("/me "))
    {
        msg = msg.right(msg.length() - 4);
        emit sendAction(group->groupId, msg);
    } else {
        emit sendMessage(group->groupId, msg);
    }
}

void GroupChatForm::onUserListChanged()
{
    nusersLabel->setText(tr("%1 users in chat").arg(group->nPeers));

    QLayoutItem *child;
    while ((child = namesListLayout->takeAt(0)))
    {
        child->widget()->hide();
        delete child->widget();
        delete child;
    }

    QStringList names(group->peers.values());
    unsigned nNames = names.size();
    for (unsigned i=0; i<nNames; ++i)
    {
        QString nameStr = names[i];
        if (i!=nNames-1)
            nameStr+=", ";
        QLabel* nameLabel = new QLabel(nameStr);
        nameLabel->setObjectName("peersLabel");
        namesListLayout->addWidget(nameLabel);
    }
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

void GroupChatForm::onMicMuteToggle()
{
    if (audioInputFlag == true)
    {
        if (micButton->objectName() == "red")
        {
            Core::getInstance()->enableGroupCallMic(group->groupId);
            micButton->setObjectName("green");
        }
        else
        {
            Core::getInstance()->disableGroupCallMic(group->groupId);
            micButton->setObjectName("red");
        }

        Style::repolish(micButton);
    }
}

void GroupChatForm::onVolMuteToggle()
{
    if (audioOutputFlag == true)
    {
        if (volButton->objectName() == "red")
        {
            Core::getInstance()->enableGroupCallVol(group->groupId);
            volButton->setObjectName("green");
        }
        else
        {
            Core::getInstance()->disableGroupCallVol(group->groupId);
            volButton->setObjectName("red");
        }

        Style::repolish(volButton);
    }
}

void GroupChatForm::onCallClicked()
{
    if (!inCall)
    {
        Core::getInstance()->joinGroupCall(group->groupId);
        audioInputFlag = true;
        audioOutputFlag = true;
        callButton->setObjectName("red");
        callButton->style()->polish(callButton);
        inCall = true;
    }
    else
    {
        Core::getInstance()->leaveGroupCall(group->groupId);
        audioInputFlag = false;
        audioOutputFlag = false;
        micButton->setObjectName("green");
        micButton->style()->polish(micButton);
        volButton->setObjectName("green");
        volButton->style()->polish(volButton);
        callButton->setObjectName("green");
        callButton->style()->polish(callButton);
        inCall = false;
    }
}

void GroupChatForm::keyPressEvent(QKeyEvent* ev)
{
    qDebug() << "Press:"<<ev->key();
    // Push to talk
    if (ev->key() == Qt::Key_P && inCall)
    {
        qDebug() << "Press:"<<ev->key();
        Core* core = Core::getInstance();
        if (!core->isGroupCallMicEnabled(group->groupId))
        {
            qDebug() << "Press:"<<ev->key();
            core->enableGroupCallMic(group->groupId);
            micButton->setObjectName("green");
            micButton->style()->polish(micButton);
            Style::repolish(micButton);
        }
    }
}

void GroupChatForm::keyReleaseEvent(QKeyEvent* ev)
{
    qDebug() << "Release:"<<ev->key();
    // Push to talk
    if (ev->key() == Qt::Key_P && inCall)
    {
        Core* core = Core::getInstance();
        if (core->isGroupCallMicEnabled(group->groupId))
        {
            core->disableGroupCallMic(group->groupId);
            micButton->setObjectName("red");
            micButton->style()->polish(micButton);
            Style::repolish(micButton);
        }
    }
}
