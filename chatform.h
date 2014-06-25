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

#include "chattextedit.h"
#include "ui_widget.h"
#include "core.h"

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
    void sendFile(int32_t, QString, QByteArray);

public slots:
    void startFileSend(ToxFile* file);

private slots:
    void onSendTriggered();
    void onAttachClicked();
    void onSliderRangeChanged();

private:
    Friend* f;
    QHBoxLayout *headLayout, *mainFootLayout;
    QVBoxLayout *headTextLayout, *mainLayout;
    QGridLayout *mainChatLayout;
    QLabel *avatar, *name, *statusMessage;
    ChatTextEdit *msgEdit;
    QPushButton *sendButton, *fileButton;
    QScrollArea *chatArea;
    QWidget *main, *head, *chatAreaWidget;
    QString previousName;
    int curRow;
    bool lockSliderToBottom;
};

#endif // CHATFORM_H
