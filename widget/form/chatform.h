#ifndef CHATFORM_H
#define CHATFORM_H

#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTextEdit>
#include <QScrollArea>
#include <QTime>
#include <QPoint>

#include "widget/tool/chattextedit.h"
#include "ui_widget.h"
#include "core.h"
#include "widget/netcamview.h"

// Spacing in px inserted when the author of the last message changes
#define AUTHOR_CHANGE_SPACING 5

class Friend;

class ChatForm : public QObject
{
    Q_OBJECT
public:
    ChatForm(Friend* chatFriend);
    ~ChatForm();
    void show(Ui::Widget& ui);
    void setName(QString newName);
    void setStatusMessage(QString newMessage);
    void addFriendMessage(QString message);
    void addMessage(QString author, QString message, QString date=QTime::currentTime().toString("hh:mm"));
    void addMessage(QLabel* author, QLabel* message, QLabel* date);

signals:
    void sendMessage(int, QString);
    void sendFile(int32_t friendId, QString, QByteArray);
    void startCall(int friendId);
    void startVideoCall(int friendId, bool video);
    void answerCall(int callId);
    void hangupCall(int callId);
    void cancelCall(int callId, int friendId);

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

private slots:
    void onSendTriggered();
    void onAttachClicked();
    void onSliderRangeChanged();
    void onCallTriggered();
    void onVideoCallTriggered();
    void onAnswerCallTriggered();
    void onHangupCallTriggered();
    void onCancelCallTriggered();
    void onChatContextMenuRequested(QPoint pos);
    void onSaveLogClicked();

private:
    Friend* f;
    QHBoxLayout *headLayout, *mainFootLayout;
    QVBoxLayout *headTextLayout, *mainLayout, *footButtonsSmall;
    QGridLayout *mainChatLayout;
    QLabel *avatar, *name, *statusMessage;
    ChatTextEdit *msgEdit;
    QPushButton *sendButton, *fileButton, *emoteButton, *callButton, *videoButton;
    QScrollArea *chatArea;
    QWidget *main, *head, *chatAreaWidget;
    QString previousName;
    NetCamView* netcam;
    int curRow;
    bool lockSliderToBottom;
    int callId;
};

#endif // CHATFORM_H
