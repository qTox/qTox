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
#include <QMimeData>
#include <QFileInfo>
#include <QDragEnterEvent>
#include <QBitmap>
#include "chatform.h"
#include "src/historykeeper.h"
#include "src/widget/form/loadhistorydialog.h"
#include "src/friend.h"
#include "src/widget/friendwidget.h"
#include "src/filetransferinstance.h"
#include "src/widget/netcamview.h"
#include "src/widget/tool/chattextedit.h"
#include "src/core.h"
#include "src/widget/widget.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/croppinglabel.h"
#include "src/misc/style.h"
#include "src/misc/settings.h"
#include "src/misc/cstring.h"
#include "src/chatlog/chatmessage.h"

ChatForm::ChatForm(Friend* chatFriend)
    : f(chatFriend)
    , audioInputFlag(false)
    , audioOutputFlag(false)
    , callId(0)
{
    nameLabel->setText(f->getDisplayedName());

    avatar->setPixmap(QPixmap(":/img/contact_dark.png"), Qt::transparent);

    statusMessageLabel = new CroppingLabel();
    statusMessageLabel->setObjectName("statusLabel");
    statusMessageLabel->setFont(Style::getFont(Style::Medium));
    statusMessageLabel->setMinimumHeight(Style::getFont(Style::Medium).pixelSize());

    netcam = new NetCamView();
    timer = nullptr;

    headTextLayout->addWidget(statusMessageLabel);
    headTextLayout->addStretch();
    callDuration = new QLabel();
    headTextLayout->addWidget(callDuration, 1, Qt::AlignCenter);
    callDuration->hide();    

    menu.addAction(tr("Load History..."), this, SLOT(onLoadHistory()));

    connect(Core::getInstance(), &Core::fileSendStarted, this, &ChatForm::startFileSend);
    connect(sendButton, &QPushButton::clicked, this, &ChatForm::onSendTriggered);
    connect(fileButton, &QPushButton::clicked, this, &ChatForm::onAttachClicked);
    connect(callButton, &QPushButton::clicked, this, &ChatForm::onCallTriggered);
    connect(videoButton, &QPushButton::clicked, this, &ChatForm::onVideoCallTriggered);
    connect(msgEdit, &ChatTextEdit::enterPressed, this, &ChatForm::onSendTriggered);
    connect(micButton, SIGNAL(clicked()), this, SLOT(onMicMuteToggle()));
    connect(volButton, SIGNAL(clicked()), this, SLOT(onVolMuteToggle()));
    //TODO: connect(chatWidget, &ChatAreaWidget::onFileTranfertInterract, this, &ChatForm::onFileTansBtnClicked);
    connect(Core::getInstance(), &Core::fileSendFailed, this, &ChatForm::onFileSendFailed);
    connect(this, SIGNAL(chatAreaCleared()), this, SLOT(clearReciepts()));

    setAcceptDrops(true);

    if (Settings::getInstance().getEnableLogging())
        loadHistory(QDateTime::currentDateTime().addDays(-7), true);
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

    bool isAction = msg.startsWith("/me ");
    if (isAction)
        msg = msg = msg.right(msg.length() - 4);

    QList<CString> splittedMsg = Core::splitMessage(msg);
    QDateTime timestamp = QDateTime::currentDateTime();

    for (CString& c_msg : splittedMsg)
    {
        QString qt_msg = CString::toString(c_msg.data(), c_msg.size());
        QString qt_msg_hist = qt_msg;
        if (isAction)
            qt_msg_hist = "/me " + qt_msg;

        bool status = !Settings::getInstance().getFauxOfflineMessaging();

        int id = HistoryKeeper::getInstance()->addChatEntry(f->getToxID().publicKey, qt_msg_hist,
                                                            Core::getInstance()->getSelfId().publicKey, timestamp, status);


        ChatMessage* ma = addSelfMessage(msg, isAction, timestamp, false);

        int rec;
        if (isAction)
            rec = Core::getInstance()->sendAction(f->getFriendID(), msg);
        else
            rec = Core::getInstance()->sendMessage(f->getFriendID(), msg);

        registerReceipt(rec, id, ma);
    }

    msgEdit->clear();
}

void ChatForm::onAttachClicked()
{
    QStringList paths = QFileDialog::getOpenFileNames(0,tr("Send a file"));
    if (paths.isEmpty())
        return;
    for (QString path : paths)
    {
        QFile file(path);
        if (!file.exists() || !file.open(QIODevice::ReadOnly))
            continue;
        if (file.isSequential())
        {
            QMessageBox::critical(0, tr("Bad Idea"), tr("You're trying to send a special (sequential) file, that's not going to work!"));
            file.close();
            continue;
        }
        long long filesize = file.size();
        file.close();
        QFileInfo fi(path);

        emit sendFile(f->getFriendID(), fi.fileName(), path, filesize);
    }
}

void ChatForm::startFileSend(ToxFile file)
{
    if (file.friendId != f->getFriendID())
        return;

    FileTransferInstance* fileTrans = new FileTransferInstance(file);
    ftransWidgets.insert(fileTrans->getId(), fileTrans);

    connect(Core::getInstance(), &Core::fileTransferInfo, fileTrans, &FileTransferInstance::onFileTransferInfo);
    connect(Core::getInstance(), &Core::fileTransferCancelled, fileTrans, &FileTransferInstance::onFileTransferCancelled);
    connect(Core::getInstance(), &Core::fileTransferFinished, fileTrans, &FileTransferInstance::onFileTransferFinished);
    connect(Core::getInstance(), SIGNAL(fileTransferAccepted(ToxFile)), fileTrans, SLOT(onFileTransferAccepted(ToxFile)));
    connect(Core::getInstance(), SIGNAL(fileTransferPaused(int,int,ToxFile::FileDirection)), fileTrans, SLOT(onFileTransferPaused(int,int,ToxFile::FileDirection)));
    connect(Core::getInstance(), SIGNAL(fileTransferRemotePausedUnpaused(ToxFile,bool)), fileTrans, SLOT(onFileTransferRemotePausedUnpaused(ToxFile,bool)));
    connect(Core::getInstance(), SIGNAL(fileTransferBrokenUnbroken(ToxFile, bool)), fileTrans, SLOT(onFileTransferBrokenUnbroken(ToxFile, bool)));

    QString name;
    if (!previousId.isMine())
    {
        Core* core = Core::getInstance();
        name = core->getUsername();
        previousId = core->getSelfId();
    }

    //TODO:
//    chatWidget->insertMessage(ChatActionPtr(new FileTransferAction(fileTrans, getElidedName(name),
//                                                                   QTime::currentTime().toString("hh:mm"), true)));
}

void ChatForm::onFileRecvRequest(ToxFile file)
{
    if (file.friendId != f->getFriendID())
        return;

    FileTransferInstance* fileTrans = new FileTransferInstance(file);
    ftransWidgets.insert(fileTrans->getId(), fileTrans);

    connect(Core::getInstance(), &Core::fileTransferInfo, fileTrans, &FileTransferInstance::onFileTransferInfo);
    connect(Core::getInstance(), &Core::fileTransferCancelled, fileTrans, &FileTransferInstance::onFileTransferCancelled);
    connect(Core::getInstance(), &Core::fileTransferFinished, fileTrans, &FileTransferInstance::onFileTransferFinished);
    connect(Core::getInstance(), SIGNAL(fileTransferAccepted(ToxFile)), fileTrans, SLOT(onFileTransferAccepted(ToxFile)));
    connect(Core::getInstance(), SIGNAL(fileTransferPaused(int,int,ToxFile::FileDirection)), fileTrans, SLOT(onFileTransferPaused(int,int,ToxFile::FileDirection)));
    connect(Core::getInstance(), SIGNAL(fileTransferRemotePausedUnpaused(ToxFile,bool)), fileTrans, SLOT(onFileTransferRemotePausedUnpaused(ToxFile,bool)));
    connect(Core::getInstance(), SIGNAL(fileTransferBrokenUnbroken(ToxFile, bool)), fileTrans, SLOT(onFileTransferBrokenUnbroken(ToxFile, bool)));

    Widget* w = Widget::getInstance();
    if (!w->isFriendWidgetCurActiveWidget(f)|| w->isMinimized() || !w->isActiveWindow())
    {
        w->newMessageAlert(f->getFriendWidget());
        f->setEventFlag(true);
        f->getFriendWidget()->updateStatusLight();
    }

    QString name;
    ToxID friendId = f->getToxID();
    if (friendId != previousId)
    {
        name = f->getDisplayedName();
        previousId = friendId;
    }

    //TODO: chatWidget->insertMessage(ChatActionPtr(new FileTransferAction(fileTrans, getElidedName(name),
    //                                                               QTime::currentTime().toString("hh:mm"), false)));

    if (!Settings::getInstance().getAutoAcceptDir(Core::getInstance()->getFriendAddress(f->getFriendID())).isEmpty()
     || Settings::getInstance().getAutoSaveEnabled())
        fileTrans->pressFromHtml("btnB");
}

void ChatForm::onAvInvite(int FriendId, int CallId, bool video)
{
    qDebug() << "onAvInvite";
    if (FriendId != f->getFriendID())
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
    
    //TODO: addSystemInfoMessage(tr("%1 calling").arg(f->getDisplayedName()), "white", QDateTime::currentDateTime());

    Widget* w = Widget::getInstance();
    if (!w->isFriendWidgetCurActiveWidget(f)|| w->isMinimized() || !w->isActiveWindow())
    {
        w->newMessageAlert(f->getFriendWidget());
        f->setEventFlag(true);
        f->getFriendWidget()->updateStatusLight();
    }
}

void ChatForm::onAvStart(int FriendId, int CallId, bool video)
{
    qDebug() << "onAvStart";
    if (FriendId != f->getFriendID())
        return;

    audioInputFlag = true;
    audioOutputFlag = true;
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

        netcam->show(Core::getInstance()->getVideoSourceFromCall(CallId), f->getDisplayedName());
    }
    else
    {
        callButton->setObjectName("red");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
    }
    
    startCounter();
}

void ChatForm::onAvCancel(int FriendId, int)
{
    qDebug() << "onAvCancel";
    
    if (FriendId != f->getFriendID())
        return;

    audioInputFlag = false;
    audioOutputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    volButton->setObjectName("green");
    volButton->style()->polish(volButton);
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));

    netcam->hide();
    
    //TODO: addSystemInfoMessage(tr("%1 stopped calling").arg(f->getDisplayedName()), "white", QDateTime::currentDateTime());
}

void ChatForm::onAvEnd(int FriendId, int)
{
    qDebug() << "onAvEnd";
    
    if (FriendId != f->getFriendID())
        return;

    audioInputFlag = false;
    audioOutputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    volButton->setObjectName("green");
    volButton->style()->polish(volButton);
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));

    netcam->hide();
    
    stopCounter();
}

void ChatForm::onAvRinging(int FriendId, int CallId, bool video)
{    
    qDebug() << "onAvRinging";
    if (FriendId != f->getFriendID())
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
    
    //TODO: addSystemInfoMessage(tr("Calling to %1").arg(f->getDisplayedName()), "white", QDateTime::currentDateTime());
}

void ChatForm::onAvStarting(int FriendId, int CallId, bool video)
{
    qDebug() << "onAvStarting";
    
    if (FriendId != f->getFriendID())
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

        netcam->show(Core::getInstance()->getVideoSourceFromCall(CallId), f->getDisplayedName());
    }
    else
    {
        callButton->setObjectName("red");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
    }
    
    startCounter();
}

void ChatForm::onAvEnding(int FriendId, int)
{
    qDebug() << "onAvEnding";
    
    if (FriendId != f->getFriendID())
        return;

    audioInputFlag = false;
    audioOutputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    volButton->setObjectName("green");
    volButton->style()->polish(volButton);
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
        
    stopCounter();
}

void ChatForm::onAvRequestTimeout(int FriendId, int)
{
    qDebug() << "onAvRequestTimeout";
    
    if (FriendId != f->getFriendID())
        return;

    audioInputFlag = false;
    audioOutputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    volButton->setObjectName("green");
    volButton->style()->polish(volButton);
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
    qDebug() << "onAvPeerTimeout";
    
    if (FriendId != f->getFriendID())
        return;

    audioInputFlag = false;
    audioOutputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    volButton->setObjectName("green");
    volButton->style()->polish(volButton);
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

void ChatForm::onAvRejected(int FriendId, int)
{
    qDebug() << "onAvRejected";
    
    if (FriendId != f->getFriendID())
        return;

    audioInputFlag = false;
    audioOutputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    volButton->setObjectName("green");
    volButton->style()->polish(volButton);
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
    
    //TODO: addSystemInfoMessage(tr("Call rejected"), "white", QDateTime::currentDateTime());

    netcam->hide();
}

void ChatForm::onAvMediaChange(int FriendId, int CallId, bool video)
{
    qDebug() << "onAvMediaChange";
    
    if (FriendId != f->getFriendID() || CallId != callId)
        return;

    if (video)
    {
        netcam->show(Core::getInstance()->getVideoSourceFromCall(CallId), f->getDisplayedName());
    }
    else
    {
        netcam->hide();
    }
}

void ChatForm::onAnswerCallTriggered()
{
    qDebug() << "onAnswerCallTriggered";
        
    audioInputFlag = true;
    audioOutputFlag = true;
    emit answerCall(callId);
}

void ChatForm::onHangupCallTriggered()
{    
    qDebug() << "onHangupCallTriggered";
    
    audioInputFlag = false;
    audioOutputFlag = false;
    emit hangupCall(callId);
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    volButton->setObjectName("green");
    volButton->style()->polish(volButton);
}

void ChatForm::onCallTriggered()
{
    qDebug() << "onCallTriggered";
    
    audioInputFlag = true;
    audioOutputFlag = true;
    callButton->disconnect();
    videoButton->disconnect();
    emit startCall(f->getFriendID());
}

void ChatForm::onVideoCallTriggered()
{
    qDebug() << "onVideoCallTriggered";
    
    audioInputFlag = true;
    audioOutputFlag = true;
    callButton->disconnect();
    videoButton->disconnect();
    emit startVideoCall(f->getFriendID(), true);
}

void ChatForm::onAvCallFailed(int FriendId)
{
    qDebug() << "onAvCallFailed";
    
    if (FriendId != f->getFriendID())
        return;

    audioInputFlag = false;
    audioOutputFlag = false;
    callButton->disconnect();
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
}

void ChatForm::onCancelCallTriggered()
{
    qDebug() << "onCancelCallTriggered";
    
    audioInputFlag = false;
    audioOutputFlag = false;
    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    volButton->setObjectName("green");
    volButton->style()->polish(volButton);
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
    emit cancelCall(callId, f->getFriendID());
}

void ChatForm::onMicMuteToggle()
{
    if (audioInputFlag == true)
    {
        emit micMuteToggle(callId);
        if (micButton->objectName() == "red")
            micButton->setObjectName("green");
        else
            micButton->setObjectName("red");

        Style::repolish(micButton);
    }
}

void ChatForm::onVolMuteToggle()
{
    if (audioOutputFlag == true)
    {
        emit volMuteToggle(callId);
        if (volButton->objectName() == "red")
            volButton->setObjectName("green");
        else
            volButton->setObjectName("red");

        Style::repolish(volButton);
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

void ChatForm::onFileSendFailed(int FriendId, const QString &fname)
{
    if (FriendId != f->getFriendID())
        return;

    //TODO: addSystemInfoMessage("File: \"" + fname + "\" failed to send.", "red", QDateTime::currentDateTime());
}

void ChatForm::onAvatarChange(int FriendId, const QPixmap &pic)
{
    if (FriendId != f->getFriendID())
        return;

    avatar->setPixmap(pic);
}

void ChatForm::dragEnterEvent(QDragEnterEvent *ev)
{
    if (ev->mimeData()->hasUrls())
        ev->acceptProposedAction();
}

void ChatForm::dropEvent(QDropEvent *ev)
{
    if (ev->mimeData()->hasUrls())
    {
        for (QUrl url : ev->mimeData()->urls())
        {
            QFileInfo info(url.path());

            if (info.exists())
                Core::getInstance()->sendFile(f->getFriendID(), info.fileName(), info.absoluteFilePath(), info.size());
        }
    }
}

void ChatForm::onAvatarRemoved(int FriendId)
{
    if (FriendId != f->getFriendID())
        return;

    avatar->setPixmap(QPixmap(":/img/contact_dark.png"), Qt::transparent);
}

void ChatForm::loadHistory(QDateTime since, bool processUndelivered)
{
    QDateTime now = QDateTime::currentDateTime();

    if (since > now)
        return;

    if (earliestMessage)
    {
        if (*earliestMessage < since)
            return;
        if (*earliestMessage < now)
        {
            now = *earliestMessage;
            now = now.addMSecs(-1);
        }
    }

    auto msgs = HistoryKeeper::getInstance()->getChatHistory(HistoryKeeper::ctSingle, f->getToxID().publicKey, since, now);

    ToxID storedPrevId;
    std::swap(storedPrevId, previousId);
//    QList<ChatMessage*> historyMessages;

//    QDate lastDate(1,0,0);
//    for (const auto &it : msgs)
//    {
//        // Show the date every new day
//        QDateTime msgDateTime = it.timestamp.toLocalTime();
//        QDate msgDate = msgDateTime.date();
//        if (msgDate > lastDate)
//        {
//            lastDate = msgDate;
//            //TODO: historyMessages.append(genSystemInfoAction(msgDate.toString(),"",QDateTime()));
//        }

//        // Show each messages
//        ChatMessage* ca = genMessageActionAction(ToxID::fromString(it.sender), it.message, false, msgDateTime);
//        if (it.isSent)
//        {
//            ca->markAsSent();
//        } else {
//            if (processUndelivered)
//            {
//                int rec;
//                if (ca->isAction())
//                    rec = Core::getInstance()->sendAction(f->getFriendID(), ca->getRawMessage());
//                else
//                    rec = Core::getInstance()->sendMessage(f->getFriendID(), ca->getRawMessage());
//                registerReceipt(rec, it.id, ca);
//            }
//        }
//        historyMessages.append(ca);
//    }
//    std::swap(storedPrevId, previousId);

//    int savedSliderPos = chatWidget->verticalScrollBar()->maximum() - chatWidget->verticalScrollBar()->value();

//    if (earliestMessage != nullptr)
//        *earliestMessage = since;

//    chatWidget->insertMessagesTop(historyMessages);

//    savedSliderPos = chatWidget->verticalScrollBar()->maximum() - savedSliderPos;
//    chatWidget->verticalScrollBar()->setValue(savedSliderPos);
}

void ChatForm::onLoadHistory()
{
    LoadHistoryDialog dlg;

    if (dlg.exec())
    {
        QDateTime fromTime = dlg.getFromDate();
        loadHistory(fromTime);
    }
}

void ChatForm::startCounter()
{
    if(!timer)
    {
        timer = new QTimer();
        connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
        timer->start(1000);
        timeElapsed.start();
        callDuration->show();
    }
}

void ChatForm::stopCounter()
{
    if(timer)
    {
        //TODO:
//        addSystemInfoMessage(tr("Call with %1 ended. %2").arg(f->getDisplayedName(),
//                                                              secondsToDHMS(timeElapsed.elapsed()/1000)),
//                             "white", QDateTime::currentDateTime());
        timer->stop();
        callDuration->setText("");
        callDuration->hide();
        timer = nullptr;
        delete timer;
    }
}

void ChatForm::updateTime()
{
    callDuration->setText(secondsToDHMS(timeElapsed.elapsed()/1000));
}

QString ChatForm::secondsToDHMS(quint32 duration)
{
    QString res;
    QString cD = tr("Call duration: ");
    int seconds = (int) (duration % 60);
    duration /= 60;
    int minutes = (int) (duration % 60);
    duration /= 60;
    int hours = (int) (duration % 24);
    int days = (int) (duration / 24);
    
    if(minutes == 0)
        return cD + res.sprintf("%02ds", seconds);
    
    if(hours == 0 && days == 0)
        return cD + res.sprintf("%02dm %02ds", minutes, seconds);
    
    if (days == 0)
        return cD + res.sprintf("%02dh %02dm %02ds", hours, minutes, seconds);
    //I assume no one will ever have call longer than ~30days
    return cD + res.sprintf("%dd%02dh %02dm %02ds", days, hours, minutes, seconds);
}

void ChatForm::registerReceipt(int receipt, int messageID, ChatMessage* msg)
{
    receipts[receipt] = messageID;
    undeliveredMsgs[messageID] = msg;
}

void ChatForm::dischargeReceipt(int receipt)
{
    auto it = receipts.find(receipt);
    if (it != receipts.end())
    {
        int mID = it.value();
        auto msgIt = undeliveredMsgs.find(mID);
        if (msgIt != undeliveredMsgs.end())
        {
            HistoryKeeper::getInstance()->markAsSent(mID);
            msgIt.value()->markAsSent(QDateTime::currentDateTime());
            undeliveredMsgs.erase(msgIt);
        }
        receipts.erase(it);
    }
}

void ChatForm::clearReciepts()
{
    receipts.clear();
    undeliveredMsgs.clear();
}

void ChatForm::deliverOfflineMsgs()
{
    if (!Settings::getInstance().getFauxOfflineMessaging())
        return;

    QMap<int, ChatMessage*> msgs = undeliveredMsgs;
    clearReciepts();

    for (auto iter = msgs.begin(); iter != msgs.end(); iter++)
    {
        QString messageText = iter.value()->toString();
        int rec;
        //if (iter.value()->isAction())
            ;//TODO: rec = Core::getInstance()->sendAction(f->getFriendID(), messageText);
        //else
            rec = Core::getInstance()->sendMessage(f->getFriendID(), messageText);
        registerReceipt(rec, iter.key(), iter.value());
    }
}
