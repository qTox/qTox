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

#include <QSet>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>

#include "genericchatform.h"
#include "src/core/corestructs.h"
#include "src/widget/tool/screenshotgrabber.h"

class Friend;
class FileTransferInstance;
class QPixmap;
class CallConfirmWidget;
class QHideEvent;
class QMoveEvent;
class OfflineMsgEngine;
class CoreAV;

class ChatForm : public GenericChatForm
{
    Q_OBJECT
public:
    explicit ChatForm(Friend* chatFriend);
    ~ChatForm();
    void setStatusMessage(QString newMessage);
    void loadHistory(QDateTime since, bool processUndelivered = false);

    void dischargeReceipt(int receipt);
    void setFriendTyping(bool isTyping);
    OfflineMsgEngine* getOfflineMsgEngine();

    virtual void show(ContentLayout* contentLayout) final override;

signals:
    void sendFile(uint32_t friendId, QString, QString, long long);
    void aliasChanged(const QString& alias);

public slots:
    void startFileSend(ToxFile file);
    void onFileRecvRequest(ToxFile file);
    void onAvInvite(uint32_t FriendId, bool video);
    void onAvStart(uint32_t FriendId, bool video);
    void onAvEnd(uint32_t FriendId);
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
    void onMicMuteToggle();
    void onVolMuteToggle();
    void onFileSendFailed(uint32_t FriendId, const QString &fname);
    void onFriendStatusChanged(uint32_t friendId, Status status);
    void onLoadHistory();
    void onUpdateTime();
    void onEnableCallButtons();
    void onScreenshotClicked();
    void onScreenshotTaken(const QPixmap &pixmap);
    void doScreenshot();
    void onMessageInserted();
    void onCopyStatusMessage();

private:
    void retranslateUi();
    void showOutgoingCall(bool video);
    void startCounter();
    void stopCounter();
    QString secondsToDHMS(quint32 duration);
    void enableCallButtons();
    void disableCallButtons();
    void SendMessageStr(QString msg);

protected:
    virtual GenericNetCamView* createNetcam() final override;
    // drag & drop
    virtual void dragEnterEvent(QDragEnterEvent* ev) final override;
    virtual void dropEvent(QDropEvent* ev) final override;
    virtual void hideEvent(QHideEvent* event) final override;
    virtual void showEvent(QShowEvent* event) final override;

private:
    CoreAV* coreav;
    Friend* f;
    CroppingLabel *statusMessageLabel;
    QMenu statusMessageMenu;
    QLabel *callDuration;
    QTimer *callDurationTimer;
    QTimer typingTimer;
    QTimer *disableCallButtonsTimer;
    QElapsedTimer timeElapsed;
    OfflineMsgEngine *offlineEngine;
    QAction* loadHistoryAction;
    QAction* copyStatusAction;

    ScreenshotGrabber* screenshotGrabber;
    QHash<uint, FileTransferInstance*> ftransWidgets;
    CallConfirmWidget *callConfirm;
    bool isTyping;
};

#endif // CHATFORM_H
