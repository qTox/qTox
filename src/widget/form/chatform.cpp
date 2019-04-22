/*
    Copyright © 2014-2018 by The qTox Project Contributors

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

#include "chatform.h"
#include "src/chatlog/chatlinecontentproxy.h"
#include "src/chatlog/chatlog.h"
#include "src/chatlog/chatmessage.h"
#include "src/chatlog/content/filetransferwidget.h"
#include "src/chatlog/content/text.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/corefile.h"
#include "src/model/friend.h"
#include "src/model/status.h"
#include "src/nexus.h"
#include "src/persistence/history.h"
#include "src/persistence/offlinemsgengine.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/video/netcamview.h"
#include "src/widget/chatformheader.h"
#include "src/widget/form/loadhistorydialog.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/searchform.h"
#include "src/widget/style.h"
#include "src/widget/tool/callconfirmwidget.h"
#include "src/widget/tool/chattextedit.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/tool/screenshotgrabber.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QStringBuilder>

#include <cassert>

/**
 * @brief ChatForm::incomingNotification Notify that we are called by someone.
 * @param friendId Friend that is calling us.
 *
 * @brief ChatForm::outgoingNotification Notify that we are calling someone.
 *
 * @brief stopNotification Tell others to stop notification of a call.
 */

static constexpr int CHAT_WIDGET_MIN_HEIGHT = 50;
static constexpr int SCREENSHOT_GRABBER_OPENING_DELAY = 500;
static constexpr int TYPING_NOTIFICATION_DURATION = 3000;

const QString ChatForm::ACTION_PREFIX = QStringLiteral("/me ");

namespace
{
    QString secondsToDHMS(quint32 duration)
    {
        QString res;
        QString cD = ChatForm::tr("Call duration: ");
        quint32 seconds = duration % 60;
        duration /= 60;
        quint32 minutes = duration % 60;
        duration /= 60;
        quint32 hours = duration % 24;
        quint32 days = duration / 24;

        // I assume no one will ever have call longer than a month
        if (days) {
            return cD + res.sprintf("%dd%02dh %02dm %02ds", days, hours, minutes, seconds);
        }

        if (hours) {
            return cD + res.sprintf("%02dh %02dm %02ds", hours, minutes, seconds);
        }

        if (minutes) {
            return cD + res.sprintf("%02dm %02ds", minutes, seconds);
        }

        return cD + res.sprintf("%02ds", seconds);
    }
} // namespace

ChatForm::ChatForm(Friend* chatFriend, History* history)
    : GenericChatForm(chatFriend)
    , f(chatFriend)
    , history{history}
    , isTyping{false}
    , lastCallIsVideo{false}
{
    setName(f->getDisplayedName());

    headWidget->setAvatar(QPixmap(":/img/contact_dark.svg"));

    statusMessageLabel = new CroppingLabel();
    statusMessageLabel->setObjectName("statusLabel");
    statusMessageLabel->setFont(Style::getFont(Style::Medium));
    statusMessageLabel->setMinimumHeight(Style::getFont(Style::Medium).pixelSize());
    statusMessageLabel->setTextFormat(Qt::PlainText);
    statusMessageLabel->setContextMenuPolicy(Qt::CustomContextMenu);

    offlineEngine = new OfflineMsgEngine(f);

    typingTimer.setSingleShot(true);

    callDurationTimer = nullptr;

    chatWidget->setTypingNotification(ChatMessage::createTypingNotification());
    chatWidget->setMinimumHeight(CHAT_WIDGET_MIN_HEIGHT);

    callDuration = new QLabel();
    headWidget->addWidget(statusMessageLabel);
    headWidget->addStretch();
    headWidget->addWidget(callDuration, 1, Qt::AlignCenter);
    callDuration->hide();

    loadHistoryAction = menu.addAction(QString(), this, SLOT(onLoadHistory()));
    copyStatusAction = statusMessageMenu.addAction(QString(), this, SLOT(onCopyStatusMessage()));

    exportChatAction =
        menu.addAction(QIcon::fromTheme("document-save"), QString(), this, SLOT(onExportChat()));

    const Core* core = Core::getInstance();
    const Profile* profile = Nexus::getProfile();
    const CoreFile* coreFile = core->getCoreFile();
    connect(coreFile, &CoreFile::fileReceiveRequested, this, &ChatForm::onFileRecvRequest);
    connect(profile, &Profile::friendAvatarChanged, this, &ChatForm::onAvatarChanged);
    connect(coreFile, &CoreFile::fileSendStarted, this, &ChatForm::startFileSend);
    connect(coreFile, &CoreFile::fileTransferFinished, this, &ChatForm::onFileTransferFinished);
    connect(coreFile, &CoreFile::fileTransferCancelled, this, &ChatForm::onFileTransferCancelled);
    connect(coreFile, &CoreFile::fileTransferBrokenUnbroken, this, &ChatForm::onFileTransferBrokenUnbroken);
    connect(coreFile, &CoreFile::fileSendFailed, this, &ChatForm::onFileSendFailed);
    connect(core, &Core::receiptRecieved, this, &ChatForm::onReceiptReceived);
    connect(core, &Core::friendMessageReceived, this, &ChatForm::onFriendMessageReceived);
    connect(core, &Core::friendTypingChanged, this, &ChatForm::onFriendTypingChanged);
    connect(core, &Core::friendStatusChanged, this, &ChatForm::onFriendStatusChanged);
    connect(coreFile, &CoreFile::fileNameChanged, this, &ChatForm::onFileNameChanged);


    const CoreAV* av = core->getAv();
    connect(av, &CoreAV::avInvite, this, &ChatForm::onAvInvite);
    connect(av, &CoreAV::avStart, this, &ChatForm::onAvStart);
    connect(av, &CoreAV::avEnd, this, &ChatForm::onAvEnd);

    connect(headWidget, &ChatFormHeader::callTriggered, this, &ChatForm::onCallTriggered);
    connect(headWidget, &ChatFormHeader::videoCallTriggered, this, &ChatForm::onVideoCallTriggered);
    connect(headWidget, &ChatFormHeader::micMuteToggle, this, &ChatForm::onMicMuteToggle);
    connect(headWidget, &ChatFormHeader::volMuteToggle, this, &ChatForm::onVolMuteToggle);

    connect(msgEdit, &ChatTextEdit::enterPressed, this, &ChatForm::onSendTriggered);
    connect(msgEdit, &ChatTextEdit::textChanged, this, &ChatForm::onTextEditChanged);
    connect(msgEdit, &ChatTextEdit::pasteImage, this, &ChatForm::sendImage);
    connect(statusMessageLabel, &CroppingLabel::customContextMenuRequested, this,
            [&](const QPoint& pos) {
                if (!statusMessageLabel->text().isEmpty()) {
                    QWidget* sender = static_cast<QWidget*>(this->sender());
                    statusMessageMenu.exec(sender->mapToGlobal(pos));
                }
            });

    connect(&typingTimer, &QTimer::timeout, this, [=] {
        Core::getInstance()->sendTyping(f->getId(), false);
        isTyping = false;
    });

    // reflect name changes in the header
    connect(headWidget, &ChatFormHeader::nameChanged, this,
            [=](const QString& newName) { f->setAlias(newName); });
    connect(headWidget, &ChatFormHeader::callAccepted, this,
            [this] { onAnswerCallTriggered(lastCallIsVideo); });
    connect(headWidget, &ChatFormHeader::callRejected, this, &ChatForm::onRejectCallTriggered);

    updateCallButtons();
    if (Nexus::getProfile()->isHistoryEnabled()) {
        loadHistoryDefaultNum(true);
    }

    setAcceptDrops(true);
    retranslateUi();
    Translator::registerHandler(std::bind(&ChatForm::retranslateUi, this), this);
}

ChatForm::~ChatForm()
{
    Translator::unregister(this);
    delete netcam;
    netcam = nullptr;
    delete offlineEngine;
}

void ChatForm::setStatusMessage(const QString& newMessage)
{
    statusMessageLabel->setText(newMessage);
    // for long messsages
    statusMessageLabel->setToolTip(Qt::convertFromPlainText(newMessage, Qt::WhiteSpaceNormal));
}

void ChatForm::onSendTriggered()
{
    SendMessageStr(msgEdit->toPlainText());
    msgEdit->clear();
}
void ChatForm::onFileNameChanged(const ToxPk& friendPk)
{
    if (friendPk != f->getPublicKey()) {
        return;
    }

    QMessageBox::warning(this, tr("Filename contained illegal characters"),
                         tr("Illegal characters have been changed to _ \n"
                            "so you can save the file on windows."));
}

void ChatForm::onTextEditChanged()
{
    if (!Settings::getInstance().getTypingNotification()) {
        if (isTyping) {
            isTyping = false;
            Core::getInstance()->sendTyping(f->getId(), false);
        }

        return;
    }
    bool isTypingNow = !msgEdit->toPlainText().isEmpty();
    if (isTyping != isTypingNow) {
        Core::getInstance()->sendTyping(f->getId(), isTypingNow);
        if (isTypingNow) {
            typingTimer.start(TYPING_NOTIFICATION_DURATION);
        }

        isTyping = isTypingNow;
    }
}

void ChatForm::onAttachClicked()
{
    QStringList paths = QFileDialog::getOpenFileNames(Q_NULLPTR, tr("Send a file"),
                                                      QDir::homePath(), nullptr, nullptr);

    if (paths.isEmpty()) {
        return;
    }

    Core* core = Core::getInstance();
    for (QString path : paths) {
        QFile file(path);
        QString fileName = QFileInfo(path).fileName();
        if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("Unable to open"),
                                 tr("qTox wasn't able to open %1").arg(fileName));
            continue;
        }

        file.close();
        if (file.isSequential()) {
            QMessageBox::critical(this, tr("Bad idea"),
                                  tr("You're trying to send a sequential file, "
                                     "which is not going to work!"));
            continue;
        }

        qint64 filesize = file.size();
        core->getCoreFile()->sendFile(f->getId(), fileName, path, filesize);
    }
}

void ChatForm::startFileSend(ToxFile file)
{
    if (file.friendId != f->getId()) {
        return;
    }

    QString name;
    const Core* core = Core::getInstance();
    ToxPk self = core->getSelfId().getPublicKey();
    if (previousId != self) {
        name = core->getUsername();
        previousId = self;
    }

    insertChatMessage(
        ChatMessage::createFileTransferMessage(name, file, true, QDateTime::currentDateTime()));

    if (history && Settings::getInstance().getEnableLogging()) {
        auto selfPk = Core::getInstance()->getSelfId().toString();
        auto pk = f->getPublicKey().toString();
        auto name = Core::getInstance()->getUsername();
        history->addNewFileMessage(pk, file.resumeFileId, file.fileName, file.filePath,
                                   file.filesize, selfPk, QDateTime::currentDateTime(), name);
    }

    Widget::getInstance()->updateFriendActivity(f);
}

void ChatForm::onFileTransferFinished(ToxFile file)
{
    history->setFileFinished(file.resumeFileId, true, file.filePath, file.hashGenerator->result());
}

void ChatForm::onFileTransferBrokenUnbroken(ToxFile file, bool broken)
{
    if (broken) {
        history->setFileFinished(file.resumeFileId, false, file.filePath, file.hashGenerator->result());
    }
}

void ChatForm::onFileTransferCancelled(ToxFile file)
{
    history->setFileFinished(file.resumeFileId, false, file.filePath, file.hashGenerator->result());
}

void ChatForm::onFileRecvRequest(ToxFile file)
{
    if (file.friendId != f->getId()) {
        return;
    }

    Widget::getInstance()->newFriendMessageAlert(f->getPublicKey());
    QString name;
    ToxPk friendId = f->getPublicKey();
    if (friendId != previousId) {
        name = f->getDisplayedName();
        previousId = friendId;
    }

    ChatMessage::Ptr msg =
        ChatMessage::createFileTransferMessage(name, file, false, QDateTime::currentDateTime());

    insertChatMessage(msg);

    if (history && Settings::getInstance().getEnableLogging()) {
        auto pk = f->getPublicKey().toString();
        auto name = f->getDisplayedName();
        history->addNewFileMessage(pk, file.resumeFileId, file.fileName, file.filePath,
                                   file.filesize, pk, QDateTime::currentDateTime(), name);
    }
    ChatLineContentProxy* proxy = static_cast<ChatLineContentProxy*>(msg->getContent(1));

    assert(proxy->getWidgetType() == ChatLineContentProxy::FileTransferWidgetType);
    FileTransferWidget* tfWidget = static_cast<FileTransferWidget*>(proxy->getWidget());

    const Settings& settings = Settings::getInstance();
    QString autoAcceptDir = settings.getAutoAcceptDir(f->getPublicKey());

    if (autoAcceptDir.isEmpty() && settings.getAutoSaveEnabled()) {
        autoAcceptDir = settings.getGlobalAutoAcceptDir();
    }

    auto maxAutoAcceptSize = settings.getMaxAutoAcceptSize();
    bool autoAcceptSizeCheckPassed = maxAutoAcceptSize == 0 || maxAutoAcceptSize >= file.filesize;

    if (!autoAcceptDir.isEmpty() && autoAcceptSizeCheckPassed) {
        tfWidget->autoAcceptTransfer(autoAcceptDir);
    }

    Widget::getInstance()->updateFriendActivity(f);
}

void ChatForm::onAvInvite(uint32_t friendId, bool video)
{
    if (friendId != f->getId()) {
        return;
    }

    QString displayedName = f->getDisplayedName();
    insertChatMessage(ChatMessage::createChatInfoMessage(tr("%1 calling").arg(displayedName),
                                                         ChatMessage::INFO,
                                                         QDateTime::currentDateTime()));

    auto testedFlag = video ? Settings::AutoAcceptCall::Video : Settings::AutoAcceptCall::Audio;
    // AutoAcceptCall is set for this friend
    if (Settings::getInstance().getAutoAcceptCall(f->getPublicKey()).testFlag(testedFlag)) {
        uint32_t friendId = f->getId();
        qDebug() << "automatic call answer";
        CoreAV* coreav = Core::getInstance()->getAv();
        QMetaObject::invokeMethod(coreav, "answerCall", Qt::QueuedConnection,
                                  Q_ARG(uint32_t, friendId), Q_ARG(bool, video));
        onAvStart(friendId, video);
    } else {
        headWidget->createCallConfirm(video);
        headWidget->showCallConfirm();
        lastCallIsVideo = video;
        emit incomingNotification(friendId);
    }
}

void ChatForm::onAvStart(uint32_t friendId, bool video)
{
    if (friendId != f->getId()) {
        return;
    }

    if (video) {
        showNetcam();
    } else {
        hideNetcam();
    }

    emit stopNotification();
    updateCallButtons();
    startCounter();
}

void ChatForm::onAvEnd(uint32_t friendId, bool error)
{
    if (friendId != f->getId()) {
        return;
    }

    headWidget->removeCallConfirm();
    // Fixes an OS X bug with ending a call while in full screen
    if (netcam && netcam->isFullScreen()) {
        netcam->showNormal();
    }

    emit stopNotification();
    emit endCallNotification();
    updateCallButtons();
    stopCounter(error);
    hideNetcam();
}

void ChatForm::showOutgoingCall(bool video)
{
    headWidget->showOutgoingCall(video);
    addSystemInfoMessage(tr("Calling %1").arg(f->getDisplayedName()), ChatMessage::INFO,
                         QDateTime::currentDateTime());
    emit outgoingNotification();
    Widget::getInstance()->updateFriendActivity(f);
}

void ChatForm::onAnswerCallTriggered(bool video)
{
    headWidget->removeCallConfirm();
    uint32_t friendId = f->getId();
    emit stopNotification();
    emit acceptCall(friendId);

    updateCallButtons();
    CoreAV* av = Core::getInstance()->getAv();
    if (!av->answerCall(friendId, video)) {
        updateCallButtons();
        stopCounter();
        hideNetcam();
        return;
    }

    onAvStart(friendId, av->isCallVideoEnabled(f));
}

void ChatForm::onRejectCallTriggered()
{
    headWidget->removeCallConfirm();
    emit rejectCall(f->getId());
}

void ChatForm::onCallTriggered()
{
    CoreAV* av = Core::getInstance()->getAv();
    uint32_t friendId = f->getId();
    if (av->isCallStarted(f)) {
        av->cancelCall(friendId);
    } else if (av->startCall(friendId, false)) {
        showOutgoingCall(false);
    }
}

void ChatForm::onVideoCallTriggered()
{
    CoreAV* av = Core::getInstance()->getAv();
    uint32_t friendId = f->getId();
    if (av->isCallStarted(f)) {
        // TODO: We want to activate video on the active call.
        if (av->isCallVideoEnabled(f)) {
            av->cancelCall(friendId);
        }
    } else if (av->startCall(friendId, true)) {
        showOutgoingCall(true);
    }
}

void ChatForm::updateCallButtons()
{
    CoreAV* av = Core::getInstance()->getAv();
    const bool audio = av->isCallActive(f);
    const bool video = av->isCallVideoEnabled(f);
    const bool online = f->isOnline();
    headWidget->updateCallButtons(online, audio, video);
    updateMuteMicButton();
    updateMuteVolButton();
}

void ChatForm::onMicMuteToggle()
{
    CoreAV* av = Core::getInstance()->getAv();
    av->toggleMuteCallInput(f);
    updateMuteMicButton();
}

void ChatForm::onVolMuteToggle()
{
    CoreAV* av = Core::getInstance()->getAv();
    av->toggleMuteCallOutput(f);
    updateMuteVolButton();
}

void ChatForm::searchInBegin(const QString& phrase, const ParameterSearch& parameter)
{
    disableSearchText();

    searchPoint = QPoint(1, -1);

    const bool isFirst = (parameter.period == PeriodSearch::WithTheFirst);
    const bool isAfter = (parameter.period == PeriodSearch::AfterDate);
    if (isFirst || isAfter) {
        if (isFirst || (isAfter && parameter.date < getFirstDate())) {
            const QString pk = f->getPublicKey().toString();
            if ((isFirst || parameter.date >= history->getStartDateChatHistory(pk).date())
                && loadHistory(phrase, parameter)) {

                return;
            }
        }

        onSearchDown(phrase, parameter);
    } else {
        if (parameter.period == PeriodSearch::BeforeDate && parameter.date < getFirstDate()) {
            const QString pk = f->getPublicKey().toString();
            if (parameter.date >= history->getStartDateChatHistory(pk).date()
                && loadHistory(phrase, parameter)) {
                return;
            }
        }

        onSearchUp(phrase, parameter);
    }
}

void ChatForm::onSearchUp(const QString& phrase, const ParameterSearch& parameter)
{
    if (phrase.isEmpty()) {
        disableSearchText();
    }

    QVector<ChatLine::Ptr> lines = chatWidget->getLines();
    int numLines = lines.size();

    int startLine;

    if (searchAfterLoadHistory) {
        startLine = 1;
        searchAfterLoadHistory = false;
    } else {
        startLine = numLines - searchPoint.x();
    }

    if (startLine == 0 && loadHistory(phrase, parameter)) {
        return;
    }

    const bool isSearch = searchInText(phrase, parameter, SearchDirection::Up);

    if (!isSearch) {
        const QString pk = f->getPublicKey().toString();
        const QDateTime newBaseDate =
            history->getDateWhereFindPhrase(pk, earliestMessage, phrase, parameter);

        if (!newBaseDate.isValid()) {
            emit messageNotFoundShow(SearchDirection::Up);
            return;
        }

        searchPoint.setX(numLines);
        searchAfterLoadHistory = true;
        loadHistoryByDateRange(newBaseDate);
    }
}

void ChatForm::onSearchDown(const QString& phrase, const ParameterSearch& parameter)
{
    if (!searchInText(phrase, parameter, SearchDirection::Down)) {
        emit messageNotFoundShow(SearchDirection::Down);
    }
}

void ChatForm::onFileSendFailed(uint32_t friendId, const QString& fname)
{
    if (friendId != f->getId()) {
        return;
    }

    addSystemInfoMessage(tr("Failed to send file \"%1\"").arg(fname), ChatMessage::ERROR,
                         QDateTime::currentDateTime());
}

void ChatForm::onFriendStatusChanged(uint32_t friendId, Status::Status status)
{
    // Disable call buttons if friend is offline
    if (friendId != f->getId()) {
        return;
    }

    if (!f->isOnline()) {
        // Hide the "is typing" message when a friend goes offline
        setFriendTyping(false);
    } else {
        offlineEngine->deliverOfflineMsgs();
    }

    updateCallButtons();

    if (Settings::getInstance().getStatusChangeNotificationEnabled()) {
        QString fStatus = Status::getTitle(status);
        addSystemInfoMessage(tr("%1 is now %2", "e.g. \"Dubslow is now online\"")
                                 .arg(f->getDisplayedName())
                                 .arg(fStatus),
                             ChatMessage::INFO, QDateTime::currentDateTime());
    }
}

void ChatForm::onFriendTypingChanged(quint32 friendId, bool isTyping)
{
    if (friendId == f->getId()) {
        setFriendTyping(isTyping);
    }
}

void ChatForm::onFriendNameChanged(const QString& name)
{
    if (sender() == f) {
        setName(name);
    }
}

void ChatForm::onFriendMessageReceived(quint32 friendId, const QString& message, bool isAction)
{
    if (friendId != f->getId()) {
        return;
    }

    QDateTime timestamp = QDateTime::currentDateTime();
    addMessage(f->getPublicKey(), message, timestamp, isAction);
}

void ChatForm::onStatusMessage(const QString& message)
{
    if (sender() == f) {
        setStatusMessage(message);
    }
}

void ChatForm::onReceiptReceived(quint32 friendId, ReceiptNum receipt)
{
    if (friendId == f->getId()) {
        offlineEngine->onReceiptReceived(receipt);
    }
}

void ChatForm::onAvatarChanged(const ToxPk& friendPk, const QPixmap& pic)
{
    if (friendPk != f->getPublicKey()) {
        return;
    }

    headWidget->setAvatar(pic);
}

GenericNetCamView* ChatForm::createNetcam()
{
    qDebug() << "creating netcam";
    uint32_t friendId = f->getId();
    NetCamView* view = new NetCamView(f->getPublicKey(), this);
    CoreAV* av = Core::getInstance()->getAv();
    VideoSource* source = av->getVideoSourceFromCall(friendId);
    view->show(source, f->getDisplayedName());
    connect(view, &GenericNetCamView::videoCallEnd, this, &ChatForm::onVideoCallTriggered);
    connect(view, &GenericNetCamView::volMuteToggle, this, &ChatForm::onVolMuteToggle);
    connect(view, &GenericNetCamView::micMuteToggle, this, &ChatForm::onMicMuteToggle);
    connect(view, &GenericNetCamView::videoPreviewToggle, view, &NetCamView::toggleVideoPreview);
    return view;
}

void ChatForm::dragEnterEvent(QDragEnterEvent* ev)
{
    if (ev->mimeData()->hasUrls()) {
        ev->acceptProposedAction();
    }
}

void ChatForm::dropEvent(QDropEvent* ev)
{
    if (!ev->mimeData()->hasUrls()) {
        return;
    }

    Core* core = Core::getInstance();
    for (const QUrl& url : ev->mimeData()->urls()) {
        QFileInfo info(url.path());
        QFile file(info.absoluteFilePath());

        QString urlString = url.toString();
        if (url.isValid() && !url.isLocalFile()
            && urlString.length() < static_cast<int>(tox_max_message_length())) {
            SendMessageStr(urlString);
            continue;
        }

        QString fileName = info.fileName();
        if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
            info.setFile(url.toLocalFile());
            file.setFileName(info.absoluteFilePath());
            if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
                QMessageBox::warning(this, tr("Unable to open"),
                                     tr("qTox wasn't able to open %1").arg(fileName));
                continue;
            }
        }

        file.close();
        if (file.isSequential()) {
            QMessageBox::critical(nullptr, tr("Bad idea"),
                                  tr("You're trying to send a sequential file, "
                                     "which is not going to work!"));
            continue;
        }

        if (info.exists()) {
            core->getCoreFile()->sendFile(f->getId(), fileName, info.absoluteFilePath(), info.size());
        }
    }
}

void ChatForm::clearChatArea()
{
    GenericChatForm::clearChatArea(/* confirm = */ false, /* inform = */ true);
    offlineEngine->removeAllMessages();
}

QString getMsgAuthorDispName(const ToxPk& authorPk, const QString& dispName)
{
    QString authorStr;
    const Core* core = Core::getInstance();
    bool isSelf = authorPk == core->getSelfId().getPublicKey();

    if (!dispName.isEmpty()) {
        authorStr = dispName;
    } else if (isSelf) {
        authorStr = core->getUsername();
    } else {
        authorStr = ChatForm::resolveToxPk(authorPk);
    }
    return authorStr;
}

void ChatForm::loadHistoryDefaultNum(bool processUndelivered)
{
    const QString pk = f->getPublicKey().toString();
    QList<History::HistMessage> msgs = history->getChatHistoryDefaultNum(pk);
    if (!msgs.isEmpty()) {
        earliestMessage = msgs.first().timestamp;
    }
    handleLoadedMessages(msgs, processUndelivered);
}

void ChatForm::loadHistoryByDateRange(const QDateTime& since, bool processUndelivered)
{
    QDateTime now = QDateTime::currentDateTime();
    if (since > now) {
        return;
    }

    if (!earliestMessage.isNull()) {
        if (earliestMessage < since) {
            return;
        }

        if (earliestMessage < now) {
            now = earliestMessage;
            now = now.addMSecs(-1);
        }
    }

    QString pk = f->getPublicKey().toString();
    earliestMessage = since;
    QList<History::HistMessage> msgs = history->getChatHistoryFromDate(pk, since, now);
    handleLoadedMessages(msgs, processUndelivered);
}

void ChatForm::handleLoadedMessages(QList<History::HistMessage> newHistMsgs, bool processUndelivered)
{
    ToxPk prevIdBackup = previousId;
    previousId = ToxPk{};
    QList<ChatLine::Ptr> chatLines;
    QDate lastDate(1, 0, 0);
    for (const auto& histMessage : newHistMsgs) {
        MessageMetadata const metadata = getMessageMetadata(histMessage);
        lastDate = addDateLineIfNeeded(chatLines, lastDate, histMessage, metadata);
        auto msg = chatMessageFromHistMessage(histMessage, metadata);

        if (!msg) {
            continue;
        }

        if (processUndelivered) {
            sendLoadedMessage(msg, metadata);
        }
        chatLines.append(msg);
        previousId = metadata.authorPk;
        prevMsgDateTime = metadata.msgDateTime;
    }
    previousId = prevIdBackup;
    insertChatlines(chatLines);
    if (searchAfterLoadHistory && chatLines.isEmpty()) {
        onContinueSearch();
    }
}

void ChatForm::insertChatlines(QList<ChatLine::Ptr> chatLines)
{
    QScrollBar* verticalBar = chatWidget->verticalScrollBar();
    int savedSliderPos = verticalBar->maximum() - verticalBar->value();
    chatWidget->insertChatlinesOnTop(chatLines);
    savedSliderPos = verticalBar->maximum() - savedSliderPos;
    verticalBar->setValue(savedSliderPos);
}

QDate ChatForm::addDateLineIfNeeded(QList<ChatLine::Ptr>& msgs, QDate const& lastDate,
                                    History::HistMessage const& newMessage,
                                    MessageMetadata const& metadata)
{
    // Show the date every new day
    QDate newDate = metadata.msgDateTime.date();
    if (newDate > lastDate) {
        QString dateText = newDate.toString(Settings::getInstance().getDateFormat());
        auto msg = ChatMessage::createChatInfoMessage(dateText, ChatMessage::INFO, QDateTime());
        msgs.append(msg);
        return newDate;
    }
    return lastDate;
}

ChatForm::MessageMetadata ChatForm::getMessageMetadata(History::HistMessage const& histMessage)
{
    const ToxPk authorPk = ToxId(histMessage.sender).getPublicKey();
    const QDateTime msgDateTime = histMessage.timestamp.toLocalTime();
    const bool isSelf = Core::getInstance()->getSelfId().getPublicKey() == authorPk;
    const bool needSending = !histMessage.isSent && isSelf;
    const bool isAction =
        histMessage.content.getType() == HistMessageContentType::message
        && histMessage.content.asMessage().startsWith(ACTION_PREFIX, Qt::CaseInsensitive);
    const RowId id = histMessage.id;
    return {isSelf, needSending, isAction, id, authorPk, msgDateTime};
}

ChatMessage::Ptr ChatForm::chatMessageFromHistMessage(History::HistMessage const& histMessage,
                                                      MessageMetadata const& metadata)
{
    ToxPk authorPk(ToxId(histMessage.sender).getPublicKey());
    QString authorStr = getMsgAuthorDispName(authorPk, histMessage.dispName);
    QDateTime dateTime = metadata.needSending ? QDateTime() : metadata.msgDateTime;


    ChatMessage::Ptr msg;

    switch (histMessage.content.getType()) {
    case HistMessageContentType::message: {
        ChatMessage::MessageType type = metadata.isAction ? ChatMessage::ACTION : ChatMessage::NORMAL;
        auto& message = histMessage.content.asMessage();
        QString messageText = metadata.isAction ? message.mid(ACTION_PREFIX.length()) : message;

        msg = ChatMessage::createChatMessage(authorStr, messageText, type, metadata.isSelf, dateTime);
        break;
    }
    case HistMessageContentType::file: {
        auto& file = histMessage.content.asFile();
        bool isMe = file.direction == ToxFile::SENDING;
        msg = ChatMessage::createFileTransferMessage(authorStr, file, isMe, dateTime);
        break;
    }
    default:
        qCritical() << "Invalid HistMessageContentType";
        assert(false);
    }

    if (!metadata.isAction && needsToHideName(authorPk, metadata.msgDateTime)) {
        msg->hideSender();
    }
    return msg;
}

void ChatForm::sendLoadedMessage(ChatMessage::Ptr chatMsg, MessageMetadata const& metadata)
{
    if (!metadata.needSending) {
        return;
    }

    ReceiptNum receipt;
    bool  messageSent{false};
    if (f->isOnline()) {
        Core* core = Core::getInstance();
        uint32_t friendId = f->getId();
        QString stringMsg = chatMsg->toString();
        messageSent = metadata.isAction ? core->sendAction(friendId, stringMsg, receipt)
                                        : core->sendMessage(friendId, stringMsg, receipt);
        if (!messageSent) {
            qWarning() << "Failed to send loaded message, adding to offline messaging";
        }
    }
    if (messageSent) {
        getOfflineMsgEngine()->addSentSavedMessage(receipt, metadata.id, chatMsg);
    } else {
        getOfflineMsgEngine()->addSavedMessage(metadata.id, chatMsg);
    }
}

void ChatForm::onScreenshotClicked()
{
    doScreenshot();
    // Give the window manager a moment to open the fullscreen grabber window
    QTimer::singleShot(SCREENSHOT_GRABBER_OPENING_DELAY, this, SLOT(hideFileMenu()));
}

void ChatForm::doScreenshot()
{
    // note: grabber is self-managed and will destroy itself when done
    ScreenshotGrabber* grabber = new ScreenshotGrabber;
    connect(grabber, &ScreenshotGrabber::screenshotTaken, this, &ChatForm::sendImage);
    grabber->showGrabber();
}

void ChatForm::sendImage(const QPixmap& pixmap)
{
    QDir(Settings::getInstance().getAppDataDirPath()).mkpath("images");

    // use ~ISO 8601 for screenshot timestamp, considering FS limitations
    // https://en.wikipedia.org/wiki/ISO_8601
    // Windows has to be supported, thus filename can't have `:` in it :/
    // Format should be: `qTox_Screenshot_yyyy-MM-dd HH-mm-ss.zzz.png`
    QString filepath = QString("%1images%2qTox_Image_%3.png")
                           .arg(Settings::getInstance().getAppDataDirPath())
                           .arg(QDir::separator())
                           .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss.zzz"));
    QFile file(filepath);

    if (file.open(QFile::ReadWrite)) {
        pixmap.save(&file, "PNG");
        qint64 filesize = file.size();
        file.close();
        QFileInfo fi(file);
        CoreFile* coreFile = Core::getInstance()->getCoreFile();
        coreFile->sendFile(f->getId(), fi.fileName(), fi.filePath(), filesize);
    } else {
        QMessageBox::warning(this,
                             tr("Failed to open temporary file", "Temporary file for screenshot"),
                             tr("qTox wasn't able to save the screenshot"));
    }
}

void ChatForm::onLoadHistory()
{
    if (!history) {
        return;
    }

    LoadHistoryDialog dlg(f->getPublicKey());
    if (dlg.exec()) {
        QDateTime fromTime = dlg.getFromDate();
        loadHistoryByDateRange(fromTime);
    }
}

void ChatForm::insertChatMessage(ChatMessage::Ptr msg)
{
    GenericChatForm::insertChatMessage(msg);
    if (netcam && bodySplitter->sizes()[1] == 0) {
        netcam->setShowMessages(true, true);
    }
}

void ChatForm::onCopyStatusMessage()
{
    // make sure to copy not truncated text directly from the friend
    QString text = f->getStatusMessage();
    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard) {
        clipboard->setText(text, QClipboard::Clipboard);
    }
}

void ChatForm::updateMuteMicButton()
{
    const CoreAV* av = Core::getInstance()->getAv();
    bool active = av->isCallActive(f);
    bool inputMuted = av->isCallInputMuted(f);
    headWidget->updateMuteMicButton(active, inputMuted);
    if (netcam) {
        netcam->updateMuteMicButton(inputMuted);
    }
}

void ChatForm::updateMuteVolButton()
{
    const CoreAV* av = Core::getInstance()->getAv();
    bool active = av->isCallActive(f);
    bool outputMuted = av->isCallOutputMuted(f);
    headWidget->updateMuteVolButton(active, outputMuted);
    if (netcam) {
        netcam->updateMuteVolButton(outputMuted);
    }
}

void ChatForm::startCounter()
{
    if (callDurationTimer) {
        return;
    }
    callDurationTimer = new QTimer();
    connect(callDurationTimer, &QTimer::timeout, this, &ChatForm::onUpdateTime);
    callDurationTimer->start(1000);
    timeElapsed.start();
    callDuration->show();
}

void ChatForm::stopCounter(bool error)
{
    if (!callDurationTimer) {
        return;
    }
    QString dhms = secondsToDHMS(timeElapsed.elapsed() / 1000);
    QString name = f->getDisplayedName();
    QString mess = error ? tr("Call with %1 ended unexpectedly. %2") : tr("Call with %1 ended. %2");
    // TODO: add notification once notifications are implemented

    addSystemInfoMessage(mess.arg(name, dhms), ChatMessage::INFO, QDateTime::currentDateTime());
    callDurationTimer->stop();
    callDuration->setText("");
    callDuration->hide();

    delete callDurationTimer;
    callDurationTimer = nullptr;
}

void ChatForm::onUpdateTime()
{
    callDuration->setText(secondsToDHMS(timeElapsed.elapsed() / 1000));
}

void ChatForm::setFriendTyping(bool isTyping)
{
    chatWidget->setTypingNotificationVisible(isTyping);
    Text* text = static_cast<Text*>(chatWidget->getTypingNotification()->getContent(1));
    QString typingDiv = "<div class=typing>%1</div>";
    QString name = f->getDisplayedName();
    text->setText(typingDiv.arg(tr("%1 is typing").arg(name)));
}

void ChatForm::show(ContentLayout* contentLayout)
{
    GenericChatForm::show(contentLayout);
}

void ChatForm::reloadTheme()
{
    chatWidget->setTypingNotification(ChatMessage::createTypingNotification());
    GenericChatForm::reloadTheme();
}

void ChatForm::showEvent(QShowEvent* event)
{
    GenericChatForm::showEvent(event);
}

void ChatForm::hideEvent(QHideEvent* event)
{
    GenericChatForm::hideEvent(event);
}

OfflineMsgEngine* ChatForm::getOfflineMsgEngine()
{
    return offlineEngine;
}

void ChatForm::SendMessageStr(QString msg)
{
    if (msg.isEmpty()) {
        return;
    }

    bool isAction = msg.startsWith(ACTION_PREFIX, Qt::CaseInsensitive);
    if (isAction) {
        msg.remove(0, ACTION_PREFIX.length());
    }

    QStringList splittedMsg = Core::splitMessage(msg, tox_max_message_length());
    QDateTime timestamp = QDateTime::currentDateTime();

    for (const QString& part : splittedMsg) {
        QString historyPart = part;
        if (isAction) {
            historyPart = ACTION_PREFIX + part;
        }

        ReceiptNum receipt;
        bool messageSent{false};
        if (f->isOnline()) {
            Core* core = Core::getInstance();
            uint32_t friendId = f->getId();
            messageSent = isAction ? core->sendAction(friendId, part, receipt) : core->sendMessage(friendId, part, receipt);
            if (!messageSent) {
                qCritical() << "Failed to send message, adding to offline messaging";
            }
        }

        ChatMessage::Ptr ma = createSelfMessage(part, timestamp, isAction, false);

        if (history && Settings::getInstance().getEnableLogging()) {
            auto* offMsgEngine = getOfflineMsgEngine();
            QString selfPk = Core::getInstance()->getSelfId().toString();
            QString pk = f->getPublicKey().toString();
            QString name = Core::getInstance()->getUsername();
            bool isSent = !Settings::getInstance().getFauxOfflineMessaging();
            history->addNewMessage(pk, historyPart, selfPk, timestamp, isSent, name,
                                   [messageSent, offMsgEngine, receipt, ma](RowId id) {
                                        if (messageSent) {
                                            offMsgEngine->addSentSavedMessage(receipt, id, ma);
                                        } else {
                                            offMsgEngine->addSavedMessage(id, ma);
                                        }
                                   });
        } else {
            // TODO: Make faux-offline messaging work partially with the history disabled
            ma->markAsSent(QDateTime::currentDateTime());
        }

        // set last message only when sending it
        msgEdit->setLastMessage(msg);
        Widget::getInstance()->updateFriendActivity(f);
    }
}

bool ChatForm::loadHistory(const QString& phrase, const ParameterSearch& parameter)
{
    const QString pk = f->getPublicKey().toString();
    const QDateTime newBaseDate =
        history->getDateWhereFindPhrase(pk, earliestMessage, phrase, parameter);

    if (newBaseDate.isValid() && getFirstDate().isValid() && newBaseDate.date() < getFirstDate()) {
        searchAfterLoadHistory = true;
        loadHistoryByDateRange(newBaseDate);

        return true;
    }

    return false;
}

void ChatForm::retranslateUi()
{
    loadHistoryAction->setText(tr("Load chat history..."));
    copyStatusAction->setText(tr("Copy"));
    exportChatAction->setText(tr("Export to file"));

    updateMuteMicButton();
    updateMuteVolButton();

    if (netcam) {
        netcam->setShowMessages(chatWidget->isVisible());
    }
}

void ChatForm::onExportChat()
{
    QString pk = f->getPublicKey().toString();
    QDateTime epochStart = QDateTime::fromMSecsSinceEpoch(0);
    QDateTime now = QDateTime::currentDateTime();
    QList<History::HistMessage> msgs = history->getChatHistoryFromDate(pk, epochStart, now);

    QString path = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Save chat log"));
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QString buffer;
    for (const auto& it : msgs) {
        if (it.content.getType() != HistMessageContentType::message) {
            continue;
        }
        QString timestamp = it.timestamp.time().toString("hh:mm:ss");
        QString datestamp = it.timestamp.date().toString("yyyy-MM-dd");
        ToxPk authorPk(ToxId(it.sender).getPublicKey());
        QString author = getMsgAuthorDispName(authorPk, it.dispName);

        buffer = buffer
                 % QString{datestamp % '\t' % timestamp % '\t' % author % '\t'
                           % it.content.asMessage() % '\n'};
    }
    file.write(buffer.toUtf8());
    file.close();
}
