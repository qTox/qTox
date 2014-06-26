#include "chatform.h"
#include "friend.h"
#include "widget/friendwidget.h"
#include "widget/widget.h"
#include "widget/filetransfertwidget.h"
#include <QFont>
#include <QTime>
#include <QScrollBar>
#include <QFileDialog>

ChatForm::ChatForm(Friend* chatFriend)
    : f(chatFriend), curRow{0}, lockSliderToBottom{true}
{
    main = new QWidget(), head = new QWidget(), chatAreaWidget = new QWidget();
    name = new QLabel(), avatar = new QLabel(), statusMessage = new QLabel();
    headLayout = new QHBoxLayout(), mainFootLayout = new QHBoxLayout();
    headTextLayout = new QVBoxLayout(), mainLayout = new QVBoxLayout(), footButtonsSmall = new QVBoxLayout();
    mainChatLayout = new QGridLayout();
    msgEdit = new ChatTextEdit();
    sendButton = new QPushButton(), fileButton = new QPushButton(), emoteButton = new QPushButton(), callButton = new QPushButton();
    chatArea = new QScrollArea();

    QFont bold;
    bold.setBold(true);
    name->setText(chatFriend->widget->name.text());
    name->setFont(bold);
    statusMessage->setText(chatFriend->widget->statusMessage.text());
    avatar->setPixmap(*chatFriend->widget->avatar.pixmap());

    chatAreaWidget->setLayout(mainChatLayout);
    chatArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    chatArea->setWidgetResizable(true);
    mainChatLayout->setColumnStretch(1,1);
    mainChatLayout->setSpacing(10);

    footButtonsSmall->setSpacing(2);

    msgEdit->setFixedHeight(50);
    QPalette toxgreen;
    toxgreen.setColor(QPalette::Button, QColor(107,194,96)); // Tox Green
    sendButton->setIcon(QIcon("img/button icons/sendmessage.svg"));
    sendButton->setFlat(true);
    sendButton->setPalette(toxgreen);
    sendButton->setAutoFillBackground(true);
    sendButton->setFixedSize(50, 50);
    sendButton->setIconSize(QSize(32,32));
    fileButton->setIcon(QIcon("img/button icons/attach.svg"));
    fileButton->setFlat(true);
    fileButton->setPalette(toxgreen);
    fileButton->setAutoFillBackground(true);
    fileButton->setIconSize(QSize(16,16));
    fileButton->setFixedSize(24,24);
    emoteButton->setIcon(QIcon("img/button icons/emoticon.svg"));
    emoteButton->setFlat(true);
    emoteButton->setPalette(toxgreen);
    emoteButton->setAutoFillBackground(true);
    emoteButton->setIconSize(QSize(16,16));
    emoteButton->setFixedSize(24,24);
    callButton->setIcon(QIcon("img/button icons/call.svg"));
    callButton->setFlat(true);
    callButton->setPalette(toxgreen);
    callButton->setAutoFillBackground(true);
    callButton->setIconSize(QSize(32,32));
    callButton->setFixedSize(50,50);

    main->setLayout(mainLayout);
    mainLayout->addWidget(chatArea);
    mainLayout->addLayout(mainFootLayout);
    mainLayout->setMargin(0);

    footButtonsSmall->addWidget(emoteButton);
    footButtonsSmall->addWidget(fileButton);

    mainFootLayout->addWidget(msgEdit);
    mainFootLayout->addLayout(footButtonsSmall);
    mainFootLayout->addWidget(sendButton);

    head->setLayout(headLayout);
    headLayout->addWidget(avatar);
    headLayout->addLayout(headTextLayout);
    headLayout->addStretch();
    headLayout->addWidget(callButton);

    headTextLayout->addStretch();
    headTextLayout->addWidget(name);
    headTextLayout->addWidget(statusMessage);
    headTextLayout->addStretch();

    chatArea->setWidget(chatAreaWidget);

    connect(Widget::getInstance()->getCore(), &Core::fileSendStarted, this, &ChatForm::startFileSend);
    connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(fileButton, SIGNAL(clicked()), this, SLOT(onAttachClicked()));
    connect(msgEdit, SIGNAL(enterPressed()), this, SLOT(onSendTriggered()));
    connect(chatArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(onSliderRangeChanged()));
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
            curRow++;
        }
        mainChatLayout->addWidget(author, curRow, 0);
    }
    previousName = author->text();
    mainChatLayout->addWidget(message, curRow, 1);
    mainChatLayout->addWidget(date, curRow, 3);
    mainChatLayout->setRowStretch(curRow+1, 1);
    mainChatLayout->setRowStretch(curRow, 0);
    curRow++;
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

void ChatForm::onCallReceived()
{

}

void ChatForm::onCallTriggered()
{

}
