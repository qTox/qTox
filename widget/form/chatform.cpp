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

#include <QDebug>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include "chatform.h"
#include "friend.h"
#include "widget/friendwidget.h"
#include "filetransferinstance.h"
#include "widget/tool/chataction.h"
#include "widget/netcamview.h"
#include "widget/chatareawidget.h"
#include "widget/tool/chattextedit.h"
#include "core.h"
#include "widget/widget.h"

ChatForm::ChatForm(Friend* chatFriend)
    : f(chatFriend)
{
    nameLabel->setText(f->getName());
    avatarLabel->setPixmap(QPixmap(":/img/contact_dark.png"));

    statusMessageLabel = new CroppingLabel();
    netcam = new NetCamView();

    headTextLayout->addWidget(statusMessageLabel);
    headTextLayout->addStretch();

    connect(Core::getInstance(), &Core::fileSendStarted, this, &ChatForm::startFileSend);
    connect(Core::getInstance(), &Core::videoFrameReceived, netcam, &NetCamView::updateDisplay);
    connect(sendButton, &QPushButton::clicked, this, &ChatForm::onSendTriggered);
    connect(fileButton, &QPushButton::clicked, this, &ChatForm::onAttachClicked);
    connect(callButton, &QPushButton::clicked, this, &ChatForm::onCallTriggered);
    connect(videoButton, &QPushButton::clicked, this, &ChatForm::onVideoCallTriggered);
    connect(msgEdit, &ChatTextEdit::enterPressed, this, &ChatForm::onSendTriggered);
    connect(micButton, SIGNAL(clicked()), this, SLOT(onMicMuteToggle()));
    connect(chatWidget, &ChatAreaWidget::onFileTranfertInterract, this, &ChatForm::onFileTansBtnClicked);
}

ChatForm::~ChatForm()
{
    delete netcam;
}

void ChatForm::setStatusMessage(QString newMessage)
{
    statusMessageLabel->setText(newMessage);
    statusMessageLabel->setToolTip(newMessage); // for overlength messsages
}

void ChatForm::onSendTriggered()
{
    QString msg = msgEdit->toPlainText();
    if (msg.isEmpty())
        return;
    QString name = Widget::getInstance()->getUsername();
    msgEdit->clear();
    addMessage(name, msg);
    emit sendMessage(f->friendId, msg);
}

void ChatForm::onAttachClicked()
{
    QString path = QFileDialog::getOpenFileName(0,tr("Send a file"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return;
    if (file.isSequential())
    {
        QMessageBox::critical(0, "Bad Idea", "You're trying to send a special (sequential) file, that's not going to work!");
        return;
        file.close();
    }
    long long filesize = file.size();
    file.close();
    QFileInfo fi(path);

    emit sendFile(f->friendId, fi.fileName(), path, filesize);
}

void ChatForm::startFileSend(ToxFile file)
{
    if (file.friendId != f->friendId)
        return;

    FileTransferInstance* fileTrans = new FileTransferInstance(file);
    ftransWidgets.insert(fileTrans->getId(), fileTrans);

    connect(Core::getInstance(), &Core::fileTransferInfo, fileTrans, &FileTransferInstance::onFileTransferInfo);
    connect(Core::getInstance(), &Core::fileTransferCancelled, fileTrans, &FileTransferInstance::onFileTransferCancelled);
    connect(Core::getInstance(), &Core::fileTransferFinished, fileTrans, &FileTransferInstance::onFileTransferFinished);
    connect(Core::getInstance(), SIGNAL(fileTransferAccepted(ToxFile)), fileTrans, SLOT(onFileTransferAccepted(ToxFile)));
    connect(Core::getInstance(), SIGNAL(fileTransferPaused(int,int,ToxFile::FileDirection)), fileTrans, SLOT(onFileTransferPaused(int,int,ToxFile::FileDirection)));
    connect(Core::getInstance(), SIGNAL(fileTransferRemotePausedUnpaused(ToxFile,bool)), fileTrans, SLOT(onFileTransferRemotePausedUnpaused(ToxFile,bool)));

    QString name = Widget::getInstance()->getUsername();
    if (name == previousName)
        name = "";
    previousName = Widget::getInstance()->getUsername();

    chatWidget->insertMessage(new FileTransferAction(fileTrans, name, QTime::currentTime().toString("hh:mm"), true));
}

void ChatForm::onFileRecvRequest(ToxFile file)
{
    if (file.friendId != f->friendId)
        return;

    FileTransferInstance* fileTrans = new FileTransferInstance(file);
    ftransWidgets.insert(fileTrans->getId(), fileTrans);

    connect(Core::getInstance(), &Core::fileTransferInfo, fileTrans, &FileTransferInstance::onFileTransferInfo);
    connect(Core::getInstance(), &Core::fileTransferCancelled, fileTrans, &FileTransferInstance::onFileTransferCancelled);
    connect(Core::getInstance(), &Core::fileTransferFinished, fileTrans, &FileTransferInstance::onFileTransferFinished);
    connect(Core::getInstance(), SIGNAL(fileTransferAccepted(ToxFile)), fileTrans, SLOT(onFileTransferAccepted(ToxFile)));
    connect(Core::getInstance(), SIGNAL(fileTransferPaused(int,int,ToxFile::FileDirection)), fileTrans, SLOT(onFileTransferPaused(int,int,ToxFile::FileDirection)));
    connect(Core::getInstance(), SIGNAL(fileTransferRemotePausedUnpaused(ToxFile,bool)), fileTrans, SLOT(onFileTransferRemotePausedUnpaused(ToxFile,bool)));

    Widget* w = Widget::getInstance();
    if (!w->isFriendWidgetCurActiveWidget(f)|| w->getIsWindowMinimized() || !w->isActiveWindow())
    {
        w->newMessageAlert();
        f->hasNewEvents=true;
        f->widget->updateStatusLight();
    }

    QString name = f->getName();
    if (name == previousName)
        name = "";
    previousName = f->getName();

    chatWidget->insertMessage(new FileTransferAction(fileTrans, name, QTime::currentTime().toString("hh:mm"), false));
}

void ChatForm::onAvInvite(int FriendId, int CallId, bool video)
{
    if (FriendId != f->friendId)
        return;

    callId = CallId;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("yellow");
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onAnswerCallTriggered()));
    }
    else
    {
        callButton->setObjectName("yellow");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onAnswerCallTriggered()));
    }

    Widget* w = Widget::getInstance();
    if (!w->isFriendWidgetCurActiveWidget(f)|| w->getIsWindowMinimized() || !w->isActiveWindow())
    {
        w->newMessageAlert();
        f->hasNewEvents=true;
        f->widget->updateStatusLight();
    }
}

void ChatForm::onAvStart(int FriendId, int CallId, bool video)
{
    if (FriendId != f->friendId)
        return;

    audioInputFlag = true;
    callId = CallId;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("red");
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
        netcam->show();
    }
    else
    {
        callButton->setObjectName("red");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
    }
}

void ChatForm::onAvCancel(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;

    audioInputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvEnd(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;

    audioInputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvRinging(int FriendId, int CallId, bool video)
{
    if (FriendId != f->friendId)
        return;

    callId = CallId;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("yellow");
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onCancelCallTriggered()));
    }
    else
    {
        callButton->setObjectName("yellow");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onCancelCallTriggered()));
    }
}

void ChatForm::onAvStarting(int FriendId, int, bool video)
{
    if (FriendId != f->friendId)
        return;

    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("red");
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
        netcam->show();
    }
    else
    {
        callButton->setObjectName("red");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
    }
}

void ChatForm::onAvEnding(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;

    audioInputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvRequestTimeout(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;

    audioInputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvPeerTimeout(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;

    audioInputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvMediaChange(int, int, bool video)
{
    if (video)
    {
        netcam->show();
    }
    else
    {
        netcam->hide();
    }
}

void ChatForm::onAnswerCallTriggered()
{
    audioInputFlag = true;
    emit answerCall(callId);
}

void ChatForm::onHangupCallTriggered()
{
    audioInputFlag = false;
    emit hangupCall(callId);
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
}

void ChatForm::onCallTriggered()
{
  audioInputFlag = true;
  callButton->disconnect();
  videoButton->disconnect();
  emit startCall(f->friendId);
}

void ChatForm::onVideoCallTriggered()
{
    audioInputFlag = true;
    callButton->disconnect();
    videoButton->disconnect();
    emit startVideoCall(f->friendId, true);
}

void ChatForm::onCancelCallTriggered()
{
    audioInputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
    emit cancelCall(callId, f->friendId);
}

void ChatForm::onMicMuteToggle()
{
    if (audioInputFlag == true)
    {
        emit micMuteToggle(callId);
        if (micButton->objectName() == "red")
        {
            micButton->setObjectName("green");
            micButton->style()->polish(micButton);
        }
        else
        {
            micButton->setObjectName("red");
            micButton->style()->polish(micButton);
        }
    }
}

void ChatForm::onFileTansBtnClicked(QString widgetName, QString buttonName)
{
    uint id = widgetName.toUInt();

    auto it = ftransWidgets.find(id);
    if (it != ftransWidgets.end())
        it.value()->pressFromHtml(buttonName);
    else
        qDebug() << "no filetransferwidget: " << id;
}
