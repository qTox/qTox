/*
    Copyright © 2014-2015 by The qTox Project

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

#include <QDebug>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QMimeData>
#include <QFileInfo>
#include <QDragEnterEvent>
#include <QBitmap>
#include <QScreen>
#include <QTemporaryFile>
#include <QGuiApplication>
#include <QStyle>
#include <cassert>
#include "chatform.h"
#include "src/core/core.h"
#include "src/friend.h"
#include "src/persistence/historykeeper.h"
#include "src/widget/style.h"
#include "src/persistence/settings.h"
#include "src/core/cstring.h"
#include "src/widget/tool/callconfirmwidget.h"
#include "src/widget/friendwidget.h"
#include "src/video/netcamview.h"
#include "src/widget/form/loadhistorydialog.h"
#include "src/widget/tool/chattextedit.h"
#include "src/widget/widget.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/chatlog/chatmessage.h"
#include "src/chatlog/content/filetransferwidget.h"
#include "src/chatlog/chatlinecontentproxy.h"
#include "src/chatlog/content/text.h"
#include "src/chatlog/chatlog.h"
#include "src/persistence/offlinemsgengine.h"
#include "src/widget/tool/screenshotgrabber.h"
#include "src/widget/tool/flyoutoverlaywidget.h"
#include "src/widget/translator.h"

ChatForm::ChatForm(Friend* chatFriend)
    : f(chatFriend)
    , callId{0}, isTyping{false}
{
    nameLabel->setText(f->getDisplayedName());

    avatar->setPixmap(QPixmap(":/img/contact_dark.svg"), Qt::transparent);

    statusMessageLabel = new CroppingLabel();
    statusMessageLabel->setObjectName("statusLabel");
    statusMessageLabel->setFont(Style::getFont(Style::Medium));
    statusMessageLabel->setMinimumHeight(Style::getFont(Style::Medium).pixelSize());
    statusMessageLabel->setTextFormat(Qt::PlainText);

    callConfirm = nullptr;
    offlineEngine = new OfflineMsgEngine(f);

    typingTimer.setSingleShot(true);

    netcam = nullptr;
    callDurationTimer = nullptr;
    disableCallButtonsTimer = nullptr;

    chatWidget->setTypingNotification(ChatMessage::createTypingNotification());

    headTextLayout->addWidget(statusMessageLabel);
    headTextLayout->addStretch();
    callDuration = new QLabel();
    headTextLayout->addWidget(callDuration, 1, Qt::AlignCenter);
    callDuration->hide();

    loadHistoryAction = menu.addAction(QString(), this, SLOT(onLoadHistory()));

    connect(Core::getInstance(), &Core::fileSendStarted, this, &ChatForm::startFileSend);
    connect(sendButton, &QPushButton::clicked, this, &ChatForm::onSendTriggered);
    connect(fileButton, &QPushButton::clicked, this, &ChatForm::onAttachClicked);
    connect(screenshotButton, &QPushButton::clicked, this, &ChatForm::onScreenshotClicked);
    connect(callButton, &QPushButton::clicked, this, &ChatForm::onCallTriggered);
    connect(videoButton, &QPushButton::clicked, this, &ChatForm::onVideoCallTriggered);
    connect(msgEdit, &ChatTextEdit::enterPressed, this, &ChatForm::onSendTriggered);
    connect(msgEdit, &ChatTextEdit::textChanged, this, &ChatForm::onTextEditChanged);
    connect(Core::getInstance(), &Core::fileSendFailed, this, &ChatForm::onFileSendFailed);
    connect(this, SIGNAL(chatAreaCleared()), getOfflineMsgEngine(), SLOT(removeAllReciepts()));
    connect(&typingTimer, &QTimer::timeout, this, [=]{
        Core::getInstance()->sendTyping(f->getFriendID(), false);
        isTyping = false;
    } );
    connect(nameLabel, &CroppingLabel::editFinished, this, [=](const QString& newName)
    {
        nameLabel->setText(newName);
        emit aliasChanged(newName);
    } );

    setAcceptDrops(true);

    retranslateUi();
    Translator::registerHandler(std::bind(&ChatForm::retranslateUi, this), this);
}

ChatForm::~ChatForm()
{
    Translator::unregister(this);
    delete netcam;
    delete callConfirm;
    delete offlineEngine;
}

void ChatForm::setStatusMessage(QString newMessage)
{
    statusMessageLabel->setText(newMessage);
    statusMessageLabel->setToolTip(newMessage); // for overlength messsages
}

void ChatForm::onSendTriggered()
{
    SendMessageStr(msgEdit->toPlainText());

    msgEdit->clear();
}

void ChatForm::onTextEditChanged()
{
    if (!Settings::getInstance().isTypingNotificationEnabled())
    {
        if (isTyping)
            Core::getInstance()->sendTyping(f->getFriendID(), false);

        isTyping = false;
        return;
    }

    if (msgEdit->toPlainText().length() > 0)
    {
        typingTimer.start(3000);
        if (!isTyping)
            Core::getInstance()->sendTyping(f->getFriendID(), (isTyping = true));
    }
    else
    {
        Core::getInstance()->sendTyping(f->getFriendID(), (isTyping = false));
    }
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
        {
            QMessageBox::warning(this, tr("File not read"), tr("qTox wasn't able to open %1").arg(QFileInfo(path).fileName()));
            continue;
        }
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

    QString name;
    if (!previousId.isActiveProfile())
    {
        Core* core = Core::getInstance();
        name = core->getUsername();
        previousId = core->getSelfId();
    }

    insertChatMessage(ChatMessage::createFileTransferMessage(name, file, true, QDateTime::currentDateTime()));
}

void ChatForm::onFileRecvRequest(ToxFile file)
{
    if (file.friendId != f->getFriendID())
        return;

    Widget* w = Widget::getInstance();
    if (!w->isFriendWidgetCurActiveWidget(f)|| w->isMinimized() || !w->isActiveWindow())
    {
        w->newMessageAlert(f->getFriendWidget());
        f->setEventFlag(true);
        f->getFriendWidget()->updateStatusLight();
    }

    QString name;
    ToxId friendId = f->getToxId();
    if (friendId != previousId)
    {
        name = f->getDisplayedName();
        previousId = friendId;
    }

    ChatMessage::Ptr msg = ChatMessage::createFileTransferMessage(name, file, false, QDateTime::currentDateTime());
    insertChatMessage(msg);

    if (!Settings::getInstance().getAutoAcceptDir(f->getToxId()).isEmpty()
            || Settings::getInstance().getAutoSaveEnabled())
    {
        ChatLineContentProxy* proxy = static_cast<ChatLineContentProxy*>(msg->getContent(1));
        assert(proxy->getWidgetType() == ChatLineContentProxy::FileTransferWidgetType);
        FileTransferWidget* tfWidget = static_cast<FileTransferWidget*>(proxy->getWidget());
        tfWidget->autoAcceptTransfer(Settings::getInstance().getAutoAcceptDir(f->getToxId()));
    }
}

void ChatForm::onAvInvite(uint32_t FriendId, int CallId, bool video)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvInvite";

    callId = CallId;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callConfirm = new CallConfirmWidget(videoButton, *f);
        if (Widget::getInstance()->isFriendWidgetCurActiveWidget(f))
            callConfirm->show();

        connect(callConfirm, &CallConfirmWidget::accepted, this, &ChatForm::onAnswerCallTriggered);
        connect(callConfirm, &CallConfirmWidget::rejected, this, &ChatForm::onRejectCallTriggered);

        callButton->setObjectName("grey");
        callButton->setToolTip("");
        videoButton->setObjectName("yellow");
        videoButton->setToolTip(tr("Accept video call"));
        connect(videoButton, &QPushButton::clicked, this, &ChatForm::onAnswerCallTriggered);
    }
    else
    {
        callConfirm = new CallConfirmWidget(callButton, *f);
        if (Widget::getInstance()->isFriendWidgetCurActiveWidget(f))
            callConfirm->show();

        connect(callConfirm, &CallConfirmWidget::accepted, this, &ChatForm::onAnswerCallTriggered);
        connect(callConfirm, &CallConfirmWidget::rejected, this, &ChatForm::onRejectCallTriggered);

        callButton->setObjectName("yellow");
        callButton->setToolTip(tr("Accept audio call"));
        videoButton->setObjectName("grey");
        videoButton->setToolTip("");
        connect(callButton, &QPushButton::clicked, this, &ChatForm::onAnswerCallTriggered);
    }
    callButton->style()->polish(callButton);
    videoButton->style()->polish(videoButton);

    insertChatMessage(ChatMessage::createChatInfoMessage(tr("%1 calling").arg(f->getDisplayedName()), ChatMessage::INFO, QDateTime::currentDateTime()));

    Widget* w = Widget::getInstance();
    if (!w->isFriendWidgetCurActiveWidget(f)|| w->isMinimized() || !w->isActiveWindow())
    {
        w->newMessageAlert(f->getFriendWidget());
        f->setEventFlag(true);
        f->getFriendWidget()->updateStatusLight();
    }
}

void ChatForm::onAvStart(uint32_t FriendId, int CallId, bool video)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvStart";

    audioInputFlag = true;
    audioOutputFlag = true;
    callId = CallId;
    callButton->disconnect();
    videoButton->disconnect();

    if (video)
    {
        callButton->setObjectName("grey");
        callButton->setToolTip("");
        videoButton->setObjectName("red");
        videoButton->setToolTip(tr("End video call"));
        connect(videoButton, SIGNAL(clicked()),
                this, SLOT(onHangupCallTriggered()));

        showNetcam();
    }
    else
    {
        callButton->setObjectName("red");
        callButton->setToolTip(tr("End audio call"));
        videoButton->setObjectName("grey");
        videoButton->setToolTip("");
        connect(callButton, SIGNAL(clicked()),
                this, SLOT(onHangupCallTriggered()));
    }
    callButton->style()->polish(callButton);
    videoButton->style()->polish(videoButton);

    micButton->setObjectName("green");
    micButton->style()->polish(micButton);
    micButton->setToolTip(tr("Mute microphone"));
    volButton->setObjectName("green");
    volButton->style()->polish(volButton);
    volButton->setToolTip(tr("Mute call"));

    connect(micButton, SIGNAL(clicked()),
            this, SLOT(onMicMuteToggle()));
    connect(volButton, SIGNAL(clicked()),
            this, SLOT(onVolMuteToggle()));

    startCounter();
}

void ChatForm::onAvCancel(uint32_t FriendId, int)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvCancel";

    delete callConfirm;
    callConfirm = nullptr;

    enableCallButtons();
    stopCounter();

    hideNetcam();

    addSystemInfoMessage(tr("%1 stopped calling").arg(f->getDisplayedName()), ChatMessage::INFO, QDateTime::currentDateTime());
}

void ChatForm::onAvEnd(uint32_t FriendId, int)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvEnd";

    delete callConfirm;
    callConfirm = nullptr;

    enableCallButtons();
    stopCounter();
    hideNetcam();
}

void ChatForm::onAvRinging(uint32_t FriendId, int CallId, bool video)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvRinging";

    callId = CallId;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        callButton->setToolTip("");
        videoButton->setObjectName("yellow");
        videoButton->style()->polish(videoButton);
        videoButton->setToolTip(tr("Cancel video call"));
        connect(videoButton, SIGNAL(clicked()),
                this, SLOT(onCancelCallTriggered()));
    }
    else
    {
        callButton->setObjectName("yellow");
        callButton->style()->polish(callButton);
        callButton->setToolTip(tr("Cancel audio call"));
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        videoButton->setToolTip("");
        connect(callButton, SIGNAL(clicked()),
                this, SLOT(onCancelCallTriggered()));
    }

    addSystemInfoMessage(tr("Calling to %1").arg(f->getDisplayedName()), ChatMessage::INFO, QDateTime::currentDateTime());
}

void ChatForm::onAvStarting(uint32_t FriendId, int CallId, bool video)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvStarting";

    callId = CallId;

    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        callButton->setToolTip("");
        videoButton->setObjectName("red");
        videoButton->style()->polish(videoButton);
        videoButton->setToolTip(tr("End video call"));
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));

        showNetcam();
    }
    else
    {
        callButton->setObjectName("red");
        callButton->style()->polish(callButton);
        callButton->setToolTip(tr("End audio call"));
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        videoButton->setToolTip("");
        connect(callButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
    }

    startCounter();
}

void ChatForm::onAvEnding(uint32_t FriendId, int)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvEnding";

    delete callConfirm;
    callConfirm = nullptr;

    enableCallButtons();
    stopCounter();

    hideNetcam();
}

void ChatForm::onAvRequestTimeout(uint32_t FriendId, int)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvRequestTimeout";

    delete callConfirm;
    callConfirm = nullptr;

    enableCallButtons();
    stopCounter();

    hideNetcam();
}

void ChatForm::onAvPeerTimeout(uint32_t FriendId, int)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvPeerTimeout";

    delete callConfirm;
    callConfirm = nullptr;

    enableCallButtons();
    stopCounter();

    hideNetcam();
}

void ChatForm::onAvRejected(uint32_t FriendId, int)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvRejected";

    delete callConfirm;
    callConfirm = nullptr;

    enableCallButtons();

    insertChatMessage(ChatMessage::createChatInfoMessage(tr("Call rejected"), ChatMessage::INFO, QDateTime::currentDateTime()));

    hideNetcam();
}

void ChatForm::onAvMediaChange(uint32_t FriendId, int CallId, bool video)
{
    if (FriendId != f->getFriendID() || CallId != callId)
        return;

    qDebug() << "onAvMediaChange";

    if (video)
        showNetcam();
    else
        hideNetcam();
}

void ChatForm::onAnswerCallTriggered()
{
    qDebug() << "onAnswerCallTriggered";

    if (callConfirm)
    {
        delete callConfirm;
        callConfirm = nullptr;
    }

    audioInputFlag = true;
    audioOutputFlag = true;
    emit answerCall(callId);
}

void ChatForm::onHangupCallTriggered()
{
    qDebug() << "onHangupCallTriggered";

    //Fixes an OS X bug with ending a call while in full screen
    if (netcam && netcam->isFullScreen())
        netcam->showNormal();

    audioInputFlag = false;
    audioOutputFlag = false;
    emit hangupCall(callId);

    enableCallButtons();
}

void ChatForm::onRejectCallTriggered()
{
    qDebug() << "onRejectCallTriggered";

    if (callConfirm)
    {
        delete callConfirm;
        callConfirm = nullptr;
    }

    audioInputFlag = false;
    audioOutputFlag = false;
    emit rejectCall(callId);

    enableCallButtons();
}

void ChatForm::onCallTriggered()
{
    qDebug() << "onCallTriggered";

    audioInputFlag = true;
    audioOutputFlag = true;
    callButton->disconnect();
    videoButton->disconnect();
    emit startCall(f->getFriendID(), false);
}

void ChatForm::onVideoCallTriggered()
{
    qDebug() << "onVideoCallTriggered";

    audioInputFlag = true;
    audioOutputFlag = true;
    callButton->disconnect();
    videoButton->disconnect();
    emit startCall(f->getFriendID(), true);
}

void ChatForm::onAvCallFailed(uint32_t FriendId)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvCallFailed";

    delete callConfirm;
    callConfirm = nullptr;

    enableCallButtons();
}

void ChatForm::onCancelCallTriggered()
{
    qDebug() << "onCancelCallTriggered";

    enableCallButtons();

    hideNetcam();
    emit cancelCall(callId, f->getFriendID());
}

void ChatForm::enableCallButtons()
{
    qDebug() << "enableCallButtons";

    audioInputFlag = false;
    audioOutputFlag = false;

    micButton->setObjectName("grey");
    micButton->style()->polish(micButton);
    micButton->setToolTip("");
    micButton->disconnect();
    volButton->setObjectName("grey");
    volButton->style()->polish(volButton);
    volButton->setToolTip("");
    volButton->disconnect();

    callButton->setObjectName("grey");
    callButton->style()->polish(callButton);
    callButton->setToolTip("");
    callButton->disconnect();
    videoButton->setObjectName("grey");
    videoButton->style()->polish(videoButton);
    videoButton->setToolTip("");
    videoButton->disconnect();

    if (disableCallButtonsTimer == nullptr)
    {
        disableCallButtonsTimer = new QTimer();
        connect(disableCallButtonsTimer, SIGNAL(timeout()),
                this, SLOT(onEnableCallButtons()));
        disableCallButtonsTimer->start(1500); // 1.5sec
        qDebug() << "timer started!!";
    }

}

void ChatForm::onEnableCallButtons()
{
    qDebug() << "onEnableCallButtons";
    audioInputFlag = false;
    audioOutputFlag = false;

    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->setToolTip(tr("Start audio call"));
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->setToolTip(tr("Start video call"));

    connect(callButton, SIGNAL(clicked()),
            this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()),
            this, SLOT(onVideoCallTriggered()));

    disableCallButtonsTimer->stop();
    delete disableCallButtonsTimer;
    disableCallButtonsTimer = nullptr;
}

void ChatForm::onMicMuteToggle()
{
    if (audioInputFlag == true)
    {
        emit micMuteToggle(callId);
        if (micButton->objectName() == "red")
        {
            micButton->setObjectName("green");
            micButton->setToolTip(tr("Mute microphone"));
        }
        else
        {
            micButton->setObjectName("red");
            micButton->setToolTip(tr("Unmute microphone"));
        }

        Style::repolish(micButton);
    }
}

void ChatForm::onVolMuteToggle()
{
    if (audioOutputFlag == true)
    {
        emit volMuteToggle(callId);
        if (volButton->objectName() == "red")
        {
            volButton->setObjectName("green");
            volButton->setToolTip(tr("Mute call"));
        }
        else
        {
            volButton->setObjectName("red");
            volButton->setToolTip(tr("Unmute call"));
        }

        Style::repolish(volButton);
    }
}

void ChatForm::onFileSendFailed(uint32_t FriendId, const QString &fname)
{
    if (FriendId != f->getFriendID())
        return;

    addSystemInfoMessage(tr("Failed to send file \"%1\"").arg(fname), ChatMessage::ERROR, QDateTime::currentDateTime());
}

void ChatForm::onAvatarChange(uint32_t FriendId, const QPixmap &pic)
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

            QFile file(info.absoluteFilePath());
            if (url.isValid() && !url.isLocalFile() && (url.toString().length() < TOX_MAX_MESSAGE_LENGTH))
            {
                SendMessageStr(url.toString());
                continue;
            }
            if (!file.exists() || !file.open(QIODevice::ReadOnly))
            {
                info.setFile(url.toLocalFile());
                file.setFileName(info.absoluteFilePath());
                if (!file.exists() || !file.open(QIODevice::ReadOnly))
                {
                    QMessageBox::warning(this, tr("File not read"), tr("qTox wasn't able to open %1").arg(info.fileName()));
                    continue;
                }
            }
            if (file.isSequential())
            {
                QMessageBox::critical(0, tr("Bad Idea"), tr("You're trying to send a special (sequential) file, that's not going to work!"));
                file.close();
                continue;
            }
            file.close();

            if (info.exists())
                Core::getInstance()->sendFile(f->getFriendID(), info.fileName(), info.absoluteFilePath(), info.size());
        }
    }
}

void ChatForm::onAvatarRemoved(uint32_t FriendId)
{
    if (FriendId != f->getFriendID())
        return;

    avatar->setPixmap(QPixmap(":/img/contact_dark.svg"), Qt::transparent);
}

void ChatForm::loadHistory(QDateTime since, bool processUndelivered)
{
    QDateTime now = historyBaselineDate.addMSecs(-1);

    if (since > now)
        return;

    if (!earliestMessage.isNull())
    {
        if (earliestMessage < since)
            return;

        if (earliestMessage < now)
        {
            now = earliestMessage;
            now = now.addMSecs(-1);
        }
    }

    auto msgs = HistoryKeeper::getInstance()->getChatHistory(HistoryKeeper::ctSingle, f->getToxId().publicKey, since, now);

    ToxId storedPrevId = previousId;
    ToxId prevId;

    QList<ChatLine::Ptr> historyMessages;

    QDate lastDate(1,0,0);
    for (const auto &it : msgs)
    {
        // Show the date every new day
        QDateTime msgDateTime = it.timestamp.toLocalTime();
        QDate msgDate = msgDateTime.date();

        if (msgDate > lastDate)
        {
            lastDate = msgDate;
            historyMessages.append(ChatMessage::createChatInfoMessage(msgDate.toString(Settings::getInstance().getDateFormat()), ChatMessage::INFO, QDateTime()));
        }

        // Show each messages
        ToxId authorId = ToxId(it.sender);
        QString authorStr = authorId.isActiveProfile() ? Core::getInstance()->getUsername() : resolveToxId(authorId);
        bool isAction = it.message.startsWith("/me ", Qt::CaseInsensitive);

        ChatMessage::Ptr msg = ChatMessage::createChatMessage(authorStr,
                                                              isAction ? it.message.right(it.message.length() - 4) : it.message,
                                                              isAction ? ChatMessage::ACTION : ChatMessage::NORMAL,
                                                              authorId.isActiveProfile(),
                                                              QDateTime());

        if (!isAction && (prevId == authorId) && (prevMsgDateTime.secsTo(msgDateTime) < getChatLog()->repNameAfter) )
            msg->hideSender();

        prevId = authorId;
        prevMsgDateTime = msgDateTime;

        if (it.isSent || !authorId.isActiveProfile())
        {
            msg->markAsSent(msgDateTime);
        }
        else
        {
            if (processUndelivered)
            {
                int rec;
                if (!isAction)
                    rec = Core::getInstance()->sendMessage(f->getFriendID(), msg->toString());
                else
                    rec = Core::getInstance()->sendAction(f->getFriendID(), msg->toString());

                getOfflineMsgEngine()->registerReceipt(rec, it.id, msg);
            }
        }
        historyMessages.append(msg);
    }

    previousId = storedPrevId;
    int savedSliderPos = chatWidget->verticalScrollBar()->maximum() - chatWidget->verticalScrollBar()->value();

    earliestMessage = since;

    chatWidget->insertChatlineOnTop(historyMessages);

    savedSliderPos = chatWidget->verticalScrollBar()->maximum() - savedSliderPos;
    chatWidget->verticalScrollBar()->setValue(savedSliderPos);
}

void ChatForm::onScreenshotClicked()
{
    doScreenshot();

    // Give the window manager a moment to open the fullscreen grabber window
    QTimer::singleShot(500, this, SLOT(hideFileMenu()));
}

void ChatForm::doScreenshot()
{
    ScreenshotGrabber* screenshotGrabber = new ScreenshotGrabber(this);
    connect(screenshotGrabber, &ScreenshotGrabber::screenshotTaken, this, &ChatForm::onScreenshotTaken);
    screenshotGrabber->showGrabber();
    // Create dir for screenshots
    QDir(Settings::getInstance().getSettingsDirPath()).mkdir("screenshots");
}

void ChatForm::onScreenshotTaken(const QPixmap &pixmap) {
    QTemporaryFile file(Settings::getInstance().getSettingsDirPath()+"screenshots"+QDir::separator()+"qTox-Screenshot-XXXXXXXX.png");
	if (!file.open())
	{
	    QMessageBox::warning(this, tr("Failed to open temporary file", "Temporary file for screenshot"),
	                         tr("qTox wasn't able to save the screenshot"));
	    return;
	}

	file.setAutoRemove(false);

	pixmap.save(&file, "PNG");

	long long filesize = file.size();
	file.close();
	QFileInfo fi(file);

	emit sendFile(f->getFriendID(), fi.fileName(), fi.filePath(), filesize);
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
    if (!callDurationTimer)
    {
        callDurationTimer = new QTimer();
        connect(callDurationTimer, SIGNAL(timeout()), this, SLOT(onUpdateTime()));
        callDurationTimer->start(1000);
        timeElapsed.start();
        callDuration->show();
    }
}

void ChatForm::stopCounter()
{
    if (callDurationTimer)
    {
        addSystemInfoMessage(tr("Call with %1 ended. %2").arg(f->getDisplayedName(),secondsToDHMS(timeElapsed.elapsed()/1000)),
                             ChatMessage::INFO, QDateTime::currentDateTime());
        callDurationTimer->stop();
        callDuration->setText("");
        callDuration->hide();

        delete callDurationTimer;
        callDurationTimer = nullptr;
    }
}

void ChatForm::onUpdateTime()
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

    if (minutes == 0)
        return cD + res.sprintf("%02ds", seconds);

    if (hours == 0 && days == 0)
        return cD + res.sprintf("%02dm %02ds", minutes, seconds);

    if (days == 0)
        return cD + res.sprintf("%02dh %02dm %02ds", hours, minutes, seconds);
    //I assume no one will ever have call longer than ~30days
    return cD + res.sprintf("%dd%02dh %02dm %02ds", days, hours, minutes, seconds);
}

void ChatForm::setFriendTyping(bool isTyping)
{
    chatWidget->setTypingNotificationVisible(isTyping);

    Text* text = static_cast<Text*>(chatWidget->getTypingNotification()->getContent(1));
    text->setText("<div class=typing>" + QString("%1 is typing").arg(f->getDisplayedName()) + "</div>");
}

void ChatForm::show(Ui::MainWindow &ui)
{
    GenericChatForm::show(ui);

    if (callConfirm)
        callConfirm->show();
}

void ChatForm::showEvent(QShowEvent* event)
{
    if (callConfirm)
        callConfirm->show();

    GenericChatForm::showEvent(event);
}

void ChatForm::hideEvent(QHideEvent* event)
{
    if (callConfirm)
        callConfirm->hide();

    GenericChatForm::hideEvent(event);
}

OfflineMsgEngine *ChatForm::getOfflineMsgEngine()
{
    return offlineEngine;
}

void ChatForm::SendMessageStr(QString msg)
{
    if (msg.isEmpty())
        return;

    bool isAction = msg.startsWith("/me ", Qt::CaseInsensitive);
    if (isAction)
        msg = msg = msg.right(msg.length() - 4);

    QList<CString> splittedMsg = Core::splitMessage(msg, TOX_MAX_MESSAGE_LENGTH);
    QDateTime timestamp = QDateTime::currentDateTime();

    for (CString& c_msg : splittedMsg)
    {
        QString qt_msg = CString::toString(c_msg.data(), c_msg.size());
        QString qt_msg_hist = qt_msg;
        if (isAction)
            qt_msg_hist = "/me " + qt_msg;

        bool status = !Settings::getInstance().getFauxOfflineMessaging();

        int id = HistoryKeeper::getInstance()->addChatEntry(f->getToxId().publicKey, qt_msg_hist,
                                                            Core::getInstance()->getSelfId().publicKey, timestamp, status);

        ChatMessage::Ptr ma = addSelfMessage(qt_msg, isAction, timestamp, false);

        int rec;
        if (isAction)
            rec = Core::getInstance()->sendAction(f->getFriendID(), qt_msg);
        else
            rec = Core::getInstance()->sendMessage(f->getFriendID(), qt_msg);

        getOfflineMsgEngine()->registerReceipt(rec, id, ma);

        msgEdit->setLastMessage(msg); //set last message only when sending it
    }
}

void ChatForm::showNetcam()
{
    if (!netcam)
        netcam = new NetCamView();
    netcam->show(Core::getInstance()->getVideoSourceFromCall(callId), f->getDisplayedName());
}

void ChatForm::hideNetcam()
{
    if (!netcam)
        return;
    netcam->hide();
    delete netcam;
    netcam = nullptr;
}

void ChatForm::retranslateUi()
{
    loadHistoryAction->setText(tr("Load chat history..."));
}
