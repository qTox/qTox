#include "chatform.h"
#include "friend.h"
#include "widget/friendwidget.h"
#include "widget/widget.h"
#include "widget/filetransfertwidget.h"
#include <QFont>
#include <QTime>
#include <QScrollBar>
#include <QFileDialog>
#include <QMenu>

ChatForm::ChatForm(Friend* chatFriend)
    : f(chatFriend), curRow{0}, lockSliderToBottom{true}
{
    main = new QWidget(), head = new QWidget(), chatAreaWidget = new QWidget();
    name = new QLabel(), avatar = new QLabel(), statusMessage = new QLabel();
    headLayout = new QHBoxLayout(), mainFootLayout = new QHBoxLayout();
    headTextLayout = new QVBoxLayout(), mainLayout = new QVBoxLayout(), footButtonsSmall = new QVBoxLayout();
    mainChatLayout = new QGridLayout();
    msgEdit = new ChatTextEdit();
    sendButton = new QPushButton(), fileButton = new QPushButton(), emoteButton = new QPushButton(), callButton = new QPushButton(), videoButton = new QPushButton();
    chatArea = new QScrollArea();

    QFont bold;
    bold.setBold(true);
    name->setText(chatFriend->widget->name.text());
    name->setFont(bold);
    statusMessage->setText(chatFriend->widget->statusMessage.text());
    avatar->setPixmap(*chatFriend->widget->avatar.pixmap());

    chatAreaWidget->setLayout(mainChatLayout);
    QString chatAreaStylesheet = "";
    try
    {
        QFile f("ui/chatArea/chatArea.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream chatAreaStylesheetStream(&f);
        chatAreaStylesheet = chatAreaStylesheetStream.readAll();
    }
    catch (int e) {}
    chatArea->setStyleSheet(chatAreaStylesheet);
    chatArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    chatArea->setWidgetResizable(true);
    chatArea->setContextMenuPolicy(Qt::CustomContextMenu);
    chatArea->setFrameStyle(QFrame::NoFrame);

    mainChatLayout->setColumnStretch(1,1);
    mainChatLayout->setSpacing(10);

    footButtonsSmall->setSpacing(2);

    QString msgEditStylesheet = "";
    try
    {
        QFile f("ui/msgEdit/msgEdit.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream msgEditStylesheetStream(&f);
        msgEditStylesheet = msgEditStylesheetStream.readAll();
    }
    catch (int e) {}
    msgEdit->setStyleSheet(msgEditStylesheet);
    msgEdit->setFixedHeight(50);
    msgEdit->setFrameStyle(QFrame::NoFrame);

    QString sendButtonStylesheet = "";
    try
    {
        QFile f("ui/sendButton/sendButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream sendButtonStylesheetStream(&f);
        sendButtonStylesheet = sendButtonStylesheetStream.readAll();
    }
    catch (int e) {}
    sendButton->setStyleSheet(sendButtonStylesheet);

    QString fileButtonStylesheet = "";
    try
    {
        QFile f("ui/fileButton/fileButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream fileButtonStylesheetStream(&f);
        fileButtonStylesheet = fileButtonStylesheetStream.readAll();
    }
    catch (int e) {}
    fileButton->setStyleSheet(fileButtonStylesheet);


    QString emoteButtonStylesheet = "";
    try
    {
        QFile f("ui/emoteButton/emoteButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream emoteButtonStylesheetStream(&f);
        emoteButtonStylesheet = emoteButtonStylesheetStream.readAll();
    }
    catch (int e) {}
    emoteButton->setStyleSheet(emoteButtonStylesheet);

    QString callButtonStylesheet = "";
    try
    {
        QFile f("ui/callButton/callButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream callButtonStylesheetStream(&f);
        callButtonStylesheet = callButtonStylesheetStream.readAll();
    }
    catch (int e) {}
    callButton->setObjectName("green");
    callButton->setStyleSheet(callButtonStylesheet);

    QString videoButtonStylesheet = "";
    try
    {
        QFile f("ui/videoButton/videoButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream videoButtonStylesheetStream(&f);
        videoButtonStylesheet = videoButtonStylesheetStream.readAll();
    }
    catch (int e) {}
    videoButton->setObjectName("green");
    videoButton->setStyleSheet(videoButtonStylesheet);

    main->setLayout(mainLayout);
    mainLayout->addWidget(chatArea);
    mainLayout->addLayout(mainFootLayout);
    mainLayout->setMargin(0);

    footButtonsSmall->addWidget(emoteButton);
    footButtonsSmall->addWidget(fileButton);

    mainFootLayout->addWidget(msgEdit);
    mainFootLayout->addLayout(footButtonsSmall);
    mainFootLayout->addSpacing(5);
    mainFootLayout->addWidget(sendButton);
    mainFootLayout->setSpacing(0);

    head->setLayout(headLayout);
    headLayout->addWidget(avatar);
    headLayout->addLayout(headTextLayout);
    headLayout->addStretch();
    headLayout->addWidget(callButton);
    headLayout->addWidget(videoButton);

    headTextLayout->addStretch();
    headTextLayout->addWidget(name);
    headTextLayout->addWidget(statusMessage);
    headTextLayout->addStretch();

    chatArea->setWidget(chatAreaWidget);

    connect(Widget::getInstance()->getCore(), &Core::fileSendStarted, this, &ChatForm::startFileSend);
    connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(fileButton, SIGNAL(clicked()), this, SLOT(onAttachClicked()));
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(msgEdit, SIGNAL(enterPressed()), this, SLOT(onSendTriggered()));
    connect(chatArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(onSliderRangeChanged()));
    connect(chatArea, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
}

ChatForm::~ChatForm()
{
    delete main;
    delete head;
}

void ChatForm::show(Ui::Widget &ui)
{
    ui.mainContent->layout()->addWidget(main);
    ui.mainHead->layout()->addWidget(head);
    main->show();
    head->show();
}

void ChatForm::setName(QString newName)
{
    name->setText(newName);
}

void ChatForm::setStatusMessage(QString newMessage)
{
    statusMessage->setText(newMessage);
}

void ChatForm::onSendTriggered()
{
    QString msg = msgEdit->toPlainText();
    if (msg.isEmpty())
        return;
    QString name = Widget::getInstance()->getUsername();
    msgEdit->clear();
    addMessage(name, msg);
    emit sendMessage(f->friendId, msg);
}

void ChatForm::addFriendMessage(QString message)
{
    QLabel *msgAuthor = new QLabel(name->text());
    QLabel *msgText = new QLabel(message);
    QLabel *msgDate = new QLabel(QTime::currentTime().toString("hh:mm"));

    addMessage(msgAuthor, msgText, msgDate);
}

void ChatForm::addMessage(QString author, QString message, QString date)
{
    addMessage(new QLabel(author), new QLabel(message), new QLabel(date));
}

void ChatForm::addMessage(QLabel* author, QLabel* message, QLabel* date)
{
    QScrollBar* scroll = chatArea->verticalScrollBar();
    lockSliderToBottom = scroll && scroll->value() == scroll->maximum();
    author->setAlignment(Qt::AlignTop | Qt::AlignRight);
    date->setAlignment(Qt::AlignTop);
    message->setWordWrap(true);
    message->setTextInteractionFlags(Qt::TextBrowserInteraction);
    author->setTextInteractionFlags(Qt::TextBrowserInteraction);
    date->setTextInteractionFlags(Qt::TextBrowserInteraction);
    if (author->text() == Widget::getInstance()->getUsername())
    {
        QPalette pal;
        pal.setColor(QPalette::WindowText, Qt::gray);
        author->setPalette(pal);
        message->setPalette(pal);
    }
    if (previousName.isEmpty() || previousName != author->text())
    {
        if (curRow)
        {
            mainChatLayout->setRowStretch(curRow, 0);
            mainChatLayout->addItem(new QSpacerItem(0,AUTHOR_CHANGE_SPACING),curRow,0,1,3);
        }
        previousName = author->text();
        curRow++;
    }
    else if (curRow)// onSaveLogClicked expects 0 or 3 QLabel per line
        author->setText("");
    mainChatLayout->addWidget(author, curRow, 0);
    mainChatLayout->addWidget(message, curRow, 1);
    mainChatLayout->addWidget(date, curRow, 3);
    mainChatLayout->setRowStretch(curRow+1, 1);
    mainChatLayout->setRowStretch(curRow, 0);
    curRow++;
    author->setContextMenuPolicy(Qt::CustomContextMenu);
    message->setContextMenuPolicy(Qt::CustomContextMenu);
    date->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(author, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
    connect(message, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
    connect(date, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
}

void ChatForm::onAttachClicked()
{
    QString path = QFileDialog::getOpenFileName(0,"Send a file");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return;
    QByteArray fileData = file.readAll();
    file.close();
    QFileInfo fi(path);

    emit sendFile(f->friendId, fi.fileName(), fileData);
}

void ChatForm::onSliderRangeChanged()
{
    QScrollBar* scroll = chatArea->verticalScrollBar();
    if (lockSliderToBottom)
         scroll->setValue(scroll->maximum());
}

void ChatForm::startFileSend(ToxFile file)
{
    if (file.friendId != f->friendId)
        return;
    QLabel *author = new QLabel(Widget::getInstance()->getUsername());
    QLabel *date = new QLabel(QTime::currentTime().toString("hh:mm"));
    QScrollBar* scroll = chatArea->verticalScrollBar();
    lockSliderToBottom = scroll && scroll->value() == scroll->maximum();
    author->setAlignment(Qt::AlignTop | Qt::AlignRight);
    date->setAlignment(Qt::AlignTop);
    QPalette pal;
    pal.setColor(QPalette::WindowText, Qt::gray);
    author->setPalette(pal);
    if (previousName.isEmpty() || previousName != author->text())
    {
        if (curRow)
        {
            mainChatLayout->setRowStretch(curRow, 0);
            mainChatLayout->addItem(new QSpacerItem(0,AUTHOR_CHANGE_SPACING),curRow,0,1,3);
            curRow++;
        }
        mainChatLayout->addWidget(author, curRow, 0);
    }
    FileTransfertWidget* fileTrans = new FileTransfertWidget(file);
    previousName = author->text();
    mainChatLayout->addWidget(fileTrans, curRow, 1);
    mainChatLayout->addWidget(date, curRow, 3);
    mainChatLayout->setRowStretch(curRow+1, 1);
    mainChatLayout->setRowStretch(curRow, 0);
    curRow++;

    connect(Widget::getInstance()->getCore(), &Core::fileTransferInfo, fileTrans, &FileTransfertWidget::onFileTransferInfo);
    connect(Widget::getInstance()->getCore(), &Core::fileTransferCancelled, fileTrans, &FileTransfertWidget::onFileTransferCancelled);
    connect(Widget::getInstance()->getCore(), &Core::fileTransferFinished, fileTrans, &FileTransfertWidget::onFileTransferFinished);
}

void ChatForm::onFileRecvRequest(ToxFile file)
{
    if (file.friendId != f->friendId)
        return;
    QLabel *author = new QLabel(f->getName());
    QLabel *date = new QLabel(QTime::currentTime().toString("hh:mm"));
    QScrollBar* scroll = chatArea->verticalScrollBar();
    lockSliderToBottom = scroll && scroll->value() == scroll->maximum();
    author->setAlignment(Qt::AlignTop | Qt::AlignRight);
    date->setAlignment(Qt::AlignTop);
    if (previousName.isEmpty() || previousName != author->text())
    {
        if (curRow)
        {
            mainChatLayout->setRowStretch(curRow, 0);
            mainChatLayout->addItem(new QSpacerItem(0,AUTHOR_CHANGE_SPACING),curRow,0,1,3);
            curRow++;
        }
        mainChatLayout->addWidget(author, curRow, 0);
    }
    FileTransfertWidget* fileTrans = new FileTransfertWidget(file);
    previousName = author->text();
    mainChatLayout->addWidget(fileTrans, curRow, 1);
    mainChatLayout->addWidget(date, curRow, 3);
    mainChatLayout->setRowStretch(curRow+1, 1);
    mainChatLayout->setRowStretch(curRow, 0);
    curRow++;

    connect(Widget::getInstance()->getCore(), &Core::fileTransferInfo, fileTrans, &FileTransfertWidget::onFileTransferInfo);
    connect(Widget::getInstance()->getCore(), &Core::fileTransferCancelled, fileTrans, &FileTransfertWidget::onFileTransferCancelled);
    connect(Widget::getInstance()->getCore(), &Core::fileTransferFinished, fileTrans, &FileTransfertWidget::onFileTransferFinished);
}

void ChatForm::onAvInvite(int FriendId, int CallId)
{
    if (FriendId != f->friendId)
        return;
    callId = CallId;
    callButton->setObjectName("yellow");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onAnswerCallTriggered()));
}

void ChatForm::onAvStart(int FriendId, int CallId)
{
    if (FriendId != f->friendId)
        return;
    callId = CallId;
    callButton->setObjectName("red");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
}

void ChatForm::onAvCancel(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->setObjectName("red");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
}

void ChatForm::onAvEnd(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
}

void ChatForm::onAvRinging(int FriendId, int CallId)
{
    if (FriendId != f->friendId)
        return;
    callId = CallId;
    callButton->setObjectName("grey");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCancelCallTriggered()));
}

void ChatForm::onAvStarting(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->setObjectName("red");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
}

void ChatForm::onAvEnding(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
}

void ChatForm::onAvRequestTimeout(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
}

void ChatForm::onAvPeerTimeout(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
}

void ChatForm::onAnswerCallTriggered()
{
    emit answerCall(callId);
}

void ChatForm::onHangupCallTriggered()
{
    emit hangupCall(callId);
}

void ChatForm::onCallTriggered()
{
    callButton->disconnect();
    emit startCall(f->friendId);
}

void ChatForm::onCancelCallTriggered()
{
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    emit cancelCall(callId, f->friendId);
}

void ChatForm::onChatContextMenuRequested(QPoint pos)
{
    QWidget* sender = (QWidget*)QObject::sender();
    pos = sender->mapToGlobal(pos);
    QMenu menu;
    menu.addAction("Save chat log", this, SLOT(onSaveLogClicked()));
    menu.exec(pos);
}

void ChatForm::onSaveLogClicked()
{
    QString path = QFileDialog::getSaveFileName(0,"Save chat log");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QString log;
    QList<QLabel*> labels = chatAreaWidget->findChildren<QLabel*>();
    int i=0;
    for (QLabel* label : labels)
    {
        log += label->text();
        if (i==2)
        {
            i=0;
            log += '\n';
        }
        else
        {
            log += '\t';
            i++;
        }
    }

    file.write(log.toUtf8());
    file.close();
}
