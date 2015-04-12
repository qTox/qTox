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
#include <QTimer>

GroupChatForm::GroupChatForm(Group* chatGroup)
    : group(chatGroup), inCall{false}
{
    nusersLabel = new QLabel();

    tabber = new TabCompleter(msgEdit, group);

    fileButton->setEnabled(false);
    if (group->isAvGroupchat())
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

    nameLabel->setText(group->getGroupWidget()->getName());

    nusersLabel->setFont(Style::getFont(Style::Medium));
    nusersLabel->setText(GroupChatForm::tr("%1 users in chat","Number of users in chat").arg(group->getPeersCount()));
    nusersLabel->setObjectName("statusLabel");

    avatar->setPixmap(Style::scaleSvgImage(":/img/group_dark.svg", avatar->width(), avatar->height()), Qt::transparent);

    msgEdit->setObjectName("group");

    namesListLayout = new FlowLayout(0,5,0);
    QStringList names(group->getPeerList());
    
    for (const QString& name : names)
    {
        QLabel *l = new QLabel(name);
        l->setTextFormat(Qt::PlainText);
        namesListLayout->addWidget(l);
    }
    
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
    connect(nameLabel, &CroppingLabel::textChanged, this, [=](QString text, QString orig)
        {if (text != orig) emit groupTitleChanged(group->getGroupId(), text.left(128));} );

    setAcceptDrops(true);
}

void GroupChatForm::onSendTriggered()
{
    QString msg = msgEdit->toPlainText();
    if (msg.isEmpty())
        return;

    msgEdit->setLastMessage(msg);
    msgEdit->clear();

    if (group->getPeersCount() != 1)
    {
        if (msg.startsWith("/me "))
        {
            msg = msg.right(msg.length() - 4);
            emit sendAction(group->getGroupId(), msg);
        }
        else
            emit sendMessage(group->getGroupId(), msg);
    }
    else
        addSelfMessage(msg, msg.startsWith("/me "), QDateTime::currentDateTime(), true);
}

void GroupChatForm::onUserListChanged()
{
    nusersLabel->setText(tr("%1 users in chat").arg(group->getPeersCount()));

    QLayoutItem *child;
    while ((child = namesListLayout->takeAt(0)))
    {
        child->widget()->hide();
        delete child->widget();
        delete child;
    }
    peerLabels.clear();

    // the list needs peers in peernumber order, nameLayout needs alphabetical
    QMap<QString, QLabel*> orderizer;

    // first traverse in peer number order, storing the QLabels as necessary
    QStringList names = group->getPeerList();
    unsigned nNames = names.size();
    for (unsigned i=0; i<nNames; ++i)
    {
        peerLabels.append(new QLabel(names[i]));
        peerLabels[i]->setTextFormat(Qt::PlainText);
        orderizer[names[i]] = peerLabels[i];
        if (group->isSelfPeerNumber(i))
            peerLabels[i]->setStyleSheet("QLabel {color : green;}");
    }
    // now alphabetize and add to layout
    names.sort(Qt::CaseInsensitive);
    for (unsigned i=0; i<nNames; ++i)
    {
        QLabel* label = orderizer[names[i]];
        if (i != nNames - 1)
            label->setText(label->text() + ", ");
        namesListLayout->addWidget(label);
    }

    // Enable or disable call button
    if (group->getPeersCount() != 1)
    {
        callButton->setEnabled(true);
        callButton->setObjectName("green");
        callButton->style()->polish(callButton);
        callButton->setToolTip(tr("Start audio call"));
    }
    else
    {
        callButton->setEnabled(false);
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        callButton->setToolTip("");
    }
}

void GroupChatForm::peerAudioPlaying(int peer)
{
    peerLabels[peer]->setStyleSheet("QLabel {color : red;}");
    if (!peerAudioTimers[peer])
    {
        peerAudioTimers[peer] = new QTimer(this);
        peerAudioTimers[peer]->setSingleShot(true);
        connect(peerAudioTimers[peer], &QTimer::timeout, [=]{this->peerLabels[peer]->setStyleSheet("");
                                                             delete this->peerAudioTimers[peer];
                                                             this->peerAudioTimers[peer] = nullptr;});
    }
    peerAudioTimers[peer]->start(500);
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
        Core::getInstance()->groupInviteFriend(friendId, group->getGroupId());
    }
}

void GroupChatForm::onMicMuteToggle()
{
    if (audioInputFlag == true)
    {
        if (micButton->objectName() == "red")
        {
            Core::getInstance()->enableGroupCallMic(group->getGroupId());
            micButton->setObjectName("green");
            micButton->setToolTip(tr("Mute microphone"));
        }
        else
        {
            Core::getInstance()->disableGroupCallMic(group->getGroupId());
            micButton->setObjectName("red");
            micButton->setToolTip(tr("Unmute microphone"));
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
            Core::getInstance()->enableGroupCallVol(group->getGroupId());
            volButton->setObjectName("green");
            volButton->setToolTip(tr("Mute call"));
        }
        else
        {
            Core::getInstance()->disableGroupCallVol(group->getGroupId());
            volButton->setObjectName("red");
            volButton->setToolTip(tr("Unmute call"));
        }

        Style::repolish(volButton);
    }
}

void GroupChatForm::onCallClicked()
{
    if (!inCall)
    {
        Core::getInstance()->joinGroupCall(group->getGroupId());
        audioInputFlag = true;
        audioOutputFlag = true;
        callButton->setObjectName("red");
        callButton->style()->polish(callButton);
        callButton->setToolTip(tr("End audio call"));
        micButton->setObjectName("green");
        micButton->style()->polish(micButton);
        micButton->setToolTip(tr("Mute microphone"));
        volButton->setObjectName("green");
        volButton->style()->polish(volButton);
        volButton->setToolTip(tr("Mute call"));
        inCall = true;
    }
    else
    {
        Core::getInstance()->leaveGroupCall(group->getGroupId());
        audioInputFlag = false;
        audioOutputFlag = false;
        callButton->setObjectName("green");
        callButton->style()->polish(callButton);
        callButton->setToolTip(tr("Start audio call"));
        micButton->setObjectName("grey");
        micButton->style()->polish(micButton);
        micButton->setToolTip("");
        volButton->setObjectName("grey");
        volButton->style()->polish(volButton);
        volButton->setToolTip("");
        inCall = false;
    }
}

void GroupChatForm::keyPressEvent(QKeyEvent* ev)
{
    // Push to talk (CTRL+P)
    if (ev->key() == Qt::Key_P && (ev->modifiers() & Qt::ControlModifier) && inCall)
    {
        Core* core = Core::getInstance();
        if (!core->isGroupCallMicEnabled(group->getGroupId()))
        {
            core->enableGroupCallMic(group->getGroupId());
            micButton->setObjectName("green");
            micButton->style()->polish(micButton);
            Style::repolish(micButton);
        }
    }

    if (msgEdit->hasFocus())
        return;
}

void GroupChatForm::keyReleaseEvent(QKeyEvent* ev)
{
    // Push to talk (CTRL+P)
    if (ev->key() == Qt::Key_P && (ev->modifiers() & Qt::ControlModifier) && inCall)
    {
        Core* core = Core::getInstance();
        if (core->isGroupCallMicEnabled(group->getGroupId()))
        {
            core->disableGroupCallMic(group->getGroupId());
            micButton->setObjectName("red");
            micButton->style()->polish(micButton);
            Style::repolish(micButton);
        }
    }

    if (msgEdit->hasFocus())
        return;
}
