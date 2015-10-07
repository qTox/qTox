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

#ifndef CHATFORM_H
#define CHATFORM_H

#include "genericchatform.h"
#include "src/core/corestructs.h"
#include <QSet>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>

class Friend;
class FileTransferInstance;
class QPixmap;
class CallConfirmWidget;
class QHideEvent;
class QMoveEvent;
class OfflineMsgEngine;

class ChatForm : public GenericChatForm
{
    Q_OBJECT
public:
    ChatForm(Friend* chatFriend);
    ~ChatForm();
    void setStatusMessage(QString newMessage);
    void loadHistory(QDateTime since, bool processUndelivered = false);

    void dischargeReceipt(int receipt);
    void setFriendTyping(bool isTyping);
    OfflineMsgEngine* getOfflineMsgEngine();

    virtual void show(ContentLayout* contentLayout) final override;

signals:
    void sendFile(uint32_t friendId, QString, QString, long long);
    void startCall(uint32_t friendId, bool video);
    void answerCall(uint32_t friendId);
    void hangupCall(uint32_t friendId);
    void cancelCall(uint32_t friendId);
    void rejectCall(uint32_t friendId);
    void micMuteToggle(uint32_t friendId);
    void volMuteToggle(uint32_t friendId);
    void aliasChanged(const QString& alias);

public slots:
    void startFileSend(ToxFile file);
    void onFileRecvRequest(ToxFile file);
    void onAvInvite(uint32_t FriendId, bool video);
    void onAvStart(uint32_t FriendId, bool video);
    void onAvCancel(uint32_t FriendId);
    void onAvEnd(uint32_t FriendId);
    void onAvRinging(uint32_t FriendId, bool video);
    void onAvCallFailed(uint32_t FriendId);
    void onMicMuteToggle();
    void onVolMuteToggle();
    void onAvatarChange(uint32_t FriendId, const QPixmap& pic);
    void onAvatarRemoved(uint32_t FriendId);

private slots:
    void onSendTriggered();
    void onTextEditChanged();
    void onAttachClicked();
    void onCallTriggered();
    void onVideoCallTriggered();
    void onAnswerCallTriggered();
    void onHangupCallTriggered();
    void onCancelCallTriggered();
    void onRejectCallTriggered();
    void onFileSendFailed(uint32_t FriendId, const QString &fname);
    void onLoadHistory();
    void onUpdateTime();
    void onEnableCallButtons();
    void onScreenshotClicked();
    void onScreenshotTaken(const QPixmap &pixmap);
    void doScreenshot();
    void onMessageInserted();

private:
    void retranslateUi();

protected:
    virtual GenericNetCamView* createNetcam() final override;
    // drag & drop
    virtual void dragEnterEvent(QDragEnterEvent* ev) final override;
    virtual void dropEvent(QDropEvent* ev) final override;
    virtual void hideEvent(QHideEvent* event) final override;
    virtual void showEvent(QShowEvent* event) final override;

private:
    Friend* f;
    CroppingLabel *statusMessageLabel;
    QLabel *callDuration;
    QTimer *callDurationTimer;
    QTimer typingTimer;
    QTimer *disableCallButtonsTimer;
    QElapsedTimer timeElapsed;
    OfflineMsgEngine *offlineEngine;
    QAction* loadHistoryAction;

    QHash<uint, FileTransferInstance*> ftransWidgets;
    void startCounter();
    void stopCounter();
    QString secondsToDHMS(quint32 duration);
    CallConfirmWidget *callConfirm;
    void enableCallButtons();
    void disableCallButtons();
    bool isTyping;
    void SendMessageStr(QString msg);
};

#endif // CHATFORM_H
