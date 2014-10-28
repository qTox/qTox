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

#ifndef CHATFORM_H
#define CHATFORM_H

#include "genericchatform.h"
#include "src/corestructs.h"

struct Friend;
class FileTransferInstance;
class NetCamView;
class QPixmap;

class ChatForm : public GenericChatForm
{
    Q_OBJECT
public:
    ChatForm(Friend* chatFriend);
    ~ChatForm();
    void setStatusMessage(QString newMessage);

signals:
    void sendFile(int32_t friendId, QString, QString, long long);
    void startCall(int friendId);
    void startVideoCall(int friendId, bool video);
    void answerCall(int callId);
    void hangupCall(int callId);
    void cancelCall(int callId, int friendId);
    void micMuteToggle(int callId);
    void volMuteToggle(int callId);

public slots:
    void startFileSend(ToxFile file);
    void onFileRecvRequest(ToxFile file);
    void onAvInvite(int FriendId, int CallId, bool video);
    void onAvStart(int FriendId, int CallId, bool video);
    void onAvCancel(int FriendId, int CallId);
    void onAvEnd(int FriendId, int CallId);
    void onAvRinging(int FriendId, int CallId, bool video);
    void onAvStarting(int FriendId, int CallId, bool video);
    void onAvEnding(int FriendId, int CallId);
    void onAvRequestTimeout(int FriendId, int CallId);
    void onAvPeerTimeout(int FriendId, int CallId);
    void onAvMediaChange(int FriendId, int CallId, bool video);
    void onAvCallFailed(int FriendId);
    void onMicMuteToggle();
    void onVolMuteToggle();
    void onAvatarChange(int FriendId, const QPixmap& pic);
    void onAvatarRemoved(int FriendId);

private slots:
    void onSendTriggered();
    void onAttachClicked();
    void onCallTriggered();
    void onVideoCallTriggered();
    void onAnswerCallTriggered();
    void onHangupCallTriggered();
    void onCancelCallTriggered();
    void onFileTansBtnClicked(QString widgetName, QString buttonName);
    void onFileSendFailed(int FriendId, const QString &fname);
    void onLoadHistory();

protected:
    // drag & drop
    void dragEnterEvent(QDragEnterEvent* ev);
    void dropEvent(QDropEvent* ev);

private:
    Friend* f;
    CroppingLabel *statusMessageLabel;
    NetCamView* netcam;
    bool audioInputFlag;
    bool audioOutputFlag;
    int callId;

    QHash<uint, FileTransferInstance*> ftransWidgets;
};

#endif // CHATFORM_H
