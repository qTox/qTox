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
#include <QBoxLayout>
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
#include <QSplitter>
#include <cassert>
#include "chatform.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/friend.h"
#include "src/persistence/historykeeper.h"
#include "src/widget/style.h"
#include "src/persistence/settings.h"
#include "src/core/cstring.h"
#include "src/widget/tool/callconfirmwidget.h"
#include "src/widget/friendwidget.h"
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
#include "src/video/netcamview.h"
#include "src/persistence/offlinemsgengine.h"
#include "src/widget/tool/screenshotgrabber.h"
#include "src/widget/tool/flyoutoverlaywidget.h"
#include "src/widget/translator.h"
#include "src/video/videosource.h"
#include "src/video/camerasource.h"

ChatForm::ChatForm(Friend* chatFriend)
    : f(chatFriend)
    , isTyping{false}
{
    Core* core = Core::getInstance();
    coreav = core->getAv();

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

    callDurationTimer = nullptr;
    disableCallButtonsTimer = nullptr;

    chatWidget->setTypingNotification(ChatMessage::createTypingNotification());

    headTextLayout->addWidget(statusMessageLabel);
    headTextLayout->addStretch();
    callDuration = new QLabel();
    headTextLayout->addWidget(callDuration, 1, Qt::AlignCenter);
    callDuration->hide();

    chatWidget->setMinimumHeight(160);
    connect(this, &GenericChatForm::messageInserted, this, &ChatForm::onMessageInserted);

    loadHistoryAction = menu.addAction(QString(), this, SLOT(onLoadHistory()));

    connect(core, &Core::fileSendStarted, this, &ChatForm::startFileSend);
    connect(sendButton, &QPushButton::clicked, this, &ChatForm::onSendTriggered);
    connect(fileButton, &QPushButton::clicked, this, &ChatForm::onAttachClicked);
    connect(screenshotButton, &QPushButton::clicked, this, &ChatForm::onScreenshotClicked);
    connect(callButton, &QPushButton::clicked, this, &ChatForm::onCallTriggered);
    connect(videoButton, &QPushButton::clicked, this, &ChatForm::onVideoCallTriggered);
    connect(msgEdit, &ChatTextEdit::enterPressed, this, &ChatForm::onSendTriggered);
    connect(msgEdit, &ChatTextEdit::textChanged, this, &ChatForm::onTextEditChanged);
    connect(core, &Core::fileSendFailed, this, &ChatForm::onFileSendFailed);
    connect(this, &ChatForm::chatAreaCleared, getOfflineMsgEngine(), &OfflineMsgEngine::removeAllReciepts);
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
    QStringList paths = QFileDialog::getOpenFileNames(this,
                                                      tr("Send a file"));
    if (paths.isEmpty())
        return;

    for (QString path : paths)
    {
        QFile file(path);
        if (!file.exists() || !file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this,
                                 tr("Unable to open"),
                                 tr("qTox wasn't able to open %1").arg(QFileInfo(path).fileName()));
            continue;
        }
        if (file.isSequential())
        {
            QMessageBox::critical(this,
                                  tr("Bad idea"),
                                  tr("You're trying to send a special (sequential) file, that's not going to work!"));
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

    Widget::getInstance()->updateFriendActivity(f);
}

void ChatForm::onFileRecvRequest(ToxFile file)
{
    if (file.friendId != f->getFriendID())
        return;

    Widget::getInstance()->newFriendMessageAlert(file.friendId);

    QString name;
    ToxId friendId = f->getToxId();
    if (friendId != previousId)
    {
        name = f->getDisplayedName();
        previousId = friendId;
    }

    ChatMessage::Ptr msg = ChatMessage::createFileTransferMessage(name, file, false, QDateTime::currentDateTime());
    insertChatMessage(msg);

    ChatLineContentProxy* proxy = static_cast<ChatLineContentProxy*>(msg->getContent(1));
    assert(proxy->getWidgetType() == ChatLineContentProxy::FileTransferWidgetType);
    FileTransferWidget* tfWidget = static_cast<FileTransferWidget*>(proxy->getWidget());

    // there is auto-accept for that conact
    if (!Settings::getInstance().getAutoAcceptDir(f->getToxId()).isEmpty())
    {
        tfWidget->autoAcceptTransfer(Settings::getInstance().getAutoAcceptDir(f->getToxId()));
    }
    else if (Settings::getInstance().getAutoSaveEnabled())
    {   //global autosave to global directory
        tfWidget->autoAcceptTransfer(Settings::getInstance().getGlobalAutoAcceptDir());
    }

    Widget::getInstance()->updateFriendActivity(f);
}

void ChatForm::onAvInvite(uint32_t FriendId, bool video)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvInvite";

    disableCallButtons();
    if (video)
    {
        callConfirm = new CallConfirmWidget(videoButton, *f);
        videoButton->setObjectName("yellow");
        videoButton->setToolTip(tr("Accept video call"));
        videoButton->style()->polish(videoButton);
        connect(videoButton, &QPushButton::clicked, this, &ChatForm::onAnswerCallTriggered);
    }
    else
    {
        callConfirm = new CallConfirmWidget(callButton, *f);
        callButton->setObjectName("yellow");
        callButton->setToolTip(tr("Accept audio call"));
        callButton->style()->polish(callButton);
        connect(callButton, &QPushButton::clicked, this, &ChatForm::onAnswerCallTriggered);
    }

    if (f->getFriendWidget()->chatFormIsSet(false))
        callConfirm->show();

    connect(callConfirm, &CallConfirmWidget::accepted, this, &ChatForm::onAnswerCallTriggered);
    connect(callConfirm, &CallConfirmWidget::rejected, this, &ChatForm::onRejectCallTriggered);

    insertChatMessage(ChatMessage::createChatInfoMessage(tr("%1 calling").arg(f->getDisplayedName()),
                                                         ChatMessage::INFO,
                                                         QDateTime::currentDateTime()));

    Widget::getInstance()->newFriendMessageAlert(FriendId);
}

void ChatForm::onAvStart(uint32_t FriendId, bool video)
{
    if (FriendId != f->getFriendID())
        return;

    qDebug() << "onAvStart";

    audioInputFlag = true;
    audioOutputFlag = true;
    disableCallButtons();

    if (video)
    {
        videoButton->setObjectName("red");
        videoButton->setToolTip(tr("End video call"));
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()),
                this, SLOT(onHangupCallTriggered()));

        showNetcam();
    }
    else
    {
        callButton->setObjectName("red");
        callButton->setToolTip(tr("End audio call"));
        callButton->style()->polish(callButton);
        connect(callButton, SIGNAL(clicked()),
                this, SLOT(onHangupCallTriggered()));
        hideNetcam();
    }

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

void ChatForm::onAvEnd(uint32_t FriendId)
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

void ChatForm::showOutgoingCall(bool video)
{
    audioInputFlag = true;
    audioOutputFlag = true;

    disableCallButtons();
    if (video)
    {
        videoButton->setObjectName("yellow");
        videoButton->style()->polish(videoButton);
        videoButton->setToolTip(tr("Cancel video call"));
        connect(videoButton, &QPushButton::clicked,
                this, &ChatForm::onCancelCallTriggered);
    }
    else
    {
        callButton->setObjectName("yellow");
        callButton->style()->polish(callButton);
        callButton->setToolTip(tr("Cancel audio call"));
        connect(callButton, &QPushButton::clicked,
                this, &ChatForm::onCancelCallTriggered);
    }

    addSystemInfoMessage(tr("Calling %1").arg(f->getDisplayedName()),
                         ChatMessage::INFO,
                         QDateTime::currentDateTime());

    Widget::getInstance()->updateFriendActivity(f);
}

void ChatForm::onAnswerCallTriggered()
{
    qDebug() << "onAnswerCallTriggered";

    if (callConfirm)
    {
        delete callConfirm;
        callConfirm = nullptr;
    }

    disableCallButtons();

    if (!coreav->answerCall(f->getFriendID()))
    {
        enableCallButtons();
        stopCounter();
        hideNetcam();
        return;
    }

    onAvStart(f->getFriendID(), coreav->isCallVideoEnabled(f->getFriendID()));
}

void ChatForm::onHangupCallTriggered()
{
    qDebug() << "onHangupCallTriggered";

    //Fixes an OS X bug with ending a call while in full screen
    if (netcam && netcam->isFullScreen())
        netcam->showNormal();

    audioInputFlag = false;
    audioOutputFlag = false;
    coreav->cancelCall(f->getFriendID());

    stopCounter();
    enableCallButtons();
    hideNetcam();
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
    coreav->cancelCall(f->getFriendID());

    enableCallButtons();
    stopCounter();
}

void ChatForm::onCallTriggered()
{
    qDebug() << "onCallTriggered";

    disableCallButtons();
    if (coreav->startCall(f->getFriendID(), false))
        showOutgoingCall(false);
}

void ChatForm::onVideoCallTriggered()
{
    qDebug() << "onVideoCallTriggered";

    disableCallButtons();
    if (coreav->startCall(f->getFriendID(), true))
        showOutgoingCall(false);
}

void ChatForm::onCancelCallTriggered()
{
    qDebug() << "onCancelCallTriggered";

    if (!coreav->cancelCall(f->getFriendID()))
        qWarning() << "Failed to cancel a call! Assuming we're not in call";

    enableCallButtons();
    stopCounter();
    hideNetcam();
}

void ChatForm::enableCallButtons()
{
    qDebug() << "enableCallButtons";

    audioInputFlag = false;
    audioOutputFlag = false;

    disableCallButtons();

    if (disableCallButtonsTimer == nullptr)
    {
        disableCallButtonsTimer = new QTimer();
        connect(disableCallButtonsTimer, SIGNAL(timeout()),
                this, SLOT(onEnableCallButtons()));
        disableCallButtonsTimer->start(1500); // 1.5sec
        qDebug() << "timer started!!";
    }

}

void ChatForm::disableCallButtons()
{
    qDebug() << "disableCallButtons";

    // Prevents race enable / disable / onEnable, when it should be disabled
    if (disableCallButtonsTimer)
    {
        disableCallButtonsTimer->stop();
        delete disableCallButtonsTimer;
        disableCallButtonsTimer = nullptr;
    }

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
        coreav->micMuteToggle(f->getFriendID());
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
        coreav->volMuteToggle(f->getFriendID());
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

GenericNetCamView *ChatForm::createNetcam()
{
    qDebug() << "creating netcam";
    NetCamView* view = new NetCamView(f->getFriendID(), this);
    view->show(Core::getInstance()->getAv()->getVideoSourceFromCall(f->getFriendID()), f->getDisplayedName());
    return view;
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
                    QMessageBox::warning(this, tr("Unable to open"), tr("qTox wasn't able to open %1").arg(info.fileName()));
                    continue;
                }
            }
            if (file.isSequential())
            {
                QMessageBox::critical(0, tr("Bad idea"), tr("You're trying to send a special (sequential) file, that's not going to work!"));
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
        QString authorStr = !it.dispName.isEmpty() ? it.dispName : (authorId.isActiveProfile() ? Core::getInstance()->getUsername() : resolveToxId(authorId));
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
    // use ~ISO 8601 for screenshot timestamp, considering FS limitations
    // https://en.wikipedia.org/wiki/ISO_8601
    // Windows has to be supported, thus filename can't have `:` in it :/
    // Format should be: `qTox_Screenshot_yyyy-MM-dd HH-mm-ss.zzz.png`
    QString filepath = QString("%1screenshots%2qTox_Screenshot_%3.png")
                           .arg(Settings::getInstance().getSettingsDirPath())
                           .arg(QDir::separator())
                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss.zzz"));

    QFile file(filepath);

    if (!file.open(QFile::ReadWrite))
    {
        QMessageBox::warning(this,
                             tr("Failed to open temporary file", "Temporary file for screenshot"),
                             tr("qTox wasn't able to save the screenshot"));
        return;
    }

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

void ChatForm::onMessageInserted()
{
    if (netcam && bodySplitter->sizes()[1] == 0)
        netcam->setShowMessages(true, true);
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
    text->setText("<div class=typing>" + QString("%1 is typing").arg(f->getDisplayedName().toHtmlEscaped()) + "</div>");
}

void ChatForm::show(ContentLayout* contentLayout)
{
    GenericChatForm::show(contentLayout);

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
                                                            Core::getInstance()->getSelfId().publicKey, timestamp, status, Core::getInstance()->getUsername());

        ChatMessage::Ptr ma = addSelfMessage(qt_msg, isAction, timestamp, false);

        int rec;
        if (isAction)
            rec = Core::getInstance()->sendAction(f->getFriendID(), qt_msg);
        else
            rec = Core::getInstance()->sendMessage(f->getFriendID(), qt_msg);

        getOfflineMsgEngine()->registerReceipt(rec, id, ma);

        msgEdit->setLastMessage(msg); //set last message only when sending it

        Widget::getInstance()->updateFriendActivity(f);
    }
}

void ChatForm::retranslateUi()
{
    QString volObjectName = volButton->objectName();
    QString micObjectName = micButton->objectName();
    loadHistoryAction->setText(tr("Load chat history..."));

    if (volObjectName == QStringLiteral("green"))
        volButton->setToolTip(tr("Mute call"));
    else if (volObjectName == QStringLiteral("red"))
        volButton->setToolTip(tr("Unmute call"));

    if (micObjectName == QStringLiteral("green"))
        micButton->setToolTip(tr("Mute microphone"));
    else if (micObjectName == QStringLiteral("red"))
        micButton->setToolTip(tr("Unmute microphone"));

    if (netcam)
        netcam->setShowMessages(chatWidget->isVisible());
}
