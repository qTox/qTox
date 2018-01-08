/*
    Copyright © 2014-2015 by The qTox Project Contributors

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

#ifndef CHATFORM_H
#define CHATFORM_H

#include <QElapsedTimer>
#include <QLabel>
#include <QSet>
#include <QTimer>

#include "genericchatform.h"
#include "src/core/core.h"
#include "src/widget/tool/screenshotgrabber.h"

class CallConfirmWidget;
class FileTransferInstance;
class Friend;
class History;
class OfflineMsgEngine;
class QPixmap;
class QHideEvent;
class QMoveEvent;

class ChatForm : public GenericChatForm
{
    Q_OBJECT
public:
    ChatForm(Friend* chatFriend, History* history);
    ~ChatForm();
    void setStatusMessage(const QString& newMessage);
    void loadHistory(const QDateTime& since, bool processUndelivered = false);

    void dischargeReceipt(int receipt);
    void setFriendTyping(bool isTyping);
    OfflineMsgEngine* getOfflineMsgEngine();

    virtual void show(ContentLayout* contentLayout) final override;

    static const QString ACTION_PREFIX;

signals:

    void incomingNotification(uint32_t friendId);
    void outgoingNotification();
    void stopNotification();
    void rejectCall(uint32_t friendId);
    void acceptCall(uint32_t friendId);

public slots:
    void startFileSend(ToxFile file);
    void onFileRecvRequest(ToxFile file);
    void onAvInvite(uint32_t friendId, bool video);
    void onAvStart(uint32_t friendId, bool video);
    void onAvEnd(uint32_t friendId, bool error);
    void onAvatarChange(uint32_t friendId, const QPixmap& pic);
    void onAvatarRemoved(uint32_t friendId);

private slots:
    void clearChatArea(bool notInForm) override final;
    void onSendTriggered() override;
    void onAttachClicked() override;
    void onScreenshotClicked() override;

    void onDeliverOfflineMessages();
    void onLoadChatHistory();
    void onTextEditChanged();
    void onCallTriggered();
    void onVideoCallTriggered();
    void onAnswerCallTriggered(bool video);
    void onRejectCallTriggered();
    void onMicMuteToggle();
    void onVolMuteToggle();

    void onFileSendFailed(uint32_t friendId, const QString& fname);
    void onFriendStatusChanged(quint32 friendId, Status status);
    void onFriendTypingChanged(quint32 friendId, bool isTyping);
    void onFriendNameChanged(const QString& name);
    void onFriendMessageReceived(quint32 friendId, const QString& message, bool isAction);
    void onStatusMessage(const QString& message);
    void onReceiptReceived(quint32 friendId, int receipt);
    void onLoadHistory();
    void onUpdateTime();
    void sendImage(const QPixmap& pixmap);
    void doScreenshot();
    void onCopyStatusMessage();
    void onExportChat();

private:
    void updateMuteMicButton();
    void updateMuteVolButton();
    void retranslateUi();
    void showOutgoingCall(bool video);
    void startCounter();
    void stopCounter(bool error = false);
    void updateCallButtons();
    void SendMessageStr(QString msg);

protected:
    GenericNetCamView* createNetcam() final override;
    void insertChatMessage(ChatMessage::Ptr msg) final override;
    void dragEnterEvent(QDragEnterEvent* ev) final override;
    void dropEvent(QDropEvent* ev) final override;
    void hideEvent(QHideEvent* event) final override;
    void showEvent(QShowEvent* event) final override;

private:
    Friend* f;
    CroppingLabel* statusMessageLabel;
    QMenu statusMessageMenu;
    QLabel* callDuration;
    QTimer* callDurationTimer;
    QTimer typingTimer;
    QElapsedTimer timeElapsed;
    OfflineMsgEngine* offlineEngine;
    QAction* loadHistoryAction;
    QAction* copyStatusAction;
    QAction* exportChatAction;

    History* history;
    QHash<uint, FileTransferInstance*> ftransWidgets;
    bool isTyping;
    bool lastCallIsVideo;
};

#endif // CHATFORM_H
