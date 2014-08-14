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

#include "chatform.h"
#include "friend.h"
#include "smileypack.h"
#include "widget/friendwidget.h"
#include "widget/widget.h"
#include "widget/filetransfertwidget.h"
#include "widget/emoticonswidget.h"
#include "style.h"
#include <QFont>
#include <QTime>
#include <QScrollBar>
#include <QFileDialog>
#include <QMenu>
#include <QWidgetAction>
#include <QGridLayout>

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
    netcam = new NetCamView();

    QFont bold;
    bold.setBold(true);
    name->setText(chatFriend->widget->name.text());
    name->setFont(bold);
    statusMessage->setText(chatFriend->widget->statusMessage.text());

    // No real avatar support in toxcore, better draw a pretty picture
    //avatar->setPixmap(*chatFriend->widget->avatar.pixmap());
    avatar->setPixmap(QPixmap(":/img/contact_dark.png"));

    chatAreaWidget->setLayout(mainChatLayout);
    chatAreaWidget->setStyleSheet(Style::get(":/ui/chatArea/chatArea.css"));

    chatArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    chatArea->setWidgetResizable(true);
    chatArea->setContextMenuPolicy(Qt::CustomContextMenu);
    chatArea->setFrameStyle(QFrame::NoFrame);

    mainChatLayout->setColumnStretch(1,1);
    mainChatLayout->setSpacing(5);

    footButtonsSmall->setSpacing(2);

    msgEdit->setStyleSheet(Style::get(":/ui/msgEdit/msgEdit.css"));
    msgEdit->setFixedHeight(50);
    msgEdit->setFrameStyle(QFrame::NoFrame);

    sendButton->setStyleSheet(Style::get(":/ui/sendButton/sendButton.css"));
    fileButton->setStyleSheet(Style::get(":/ui/fileButton/fileButton.css"));
    emoteButton->setStyleSheet(Style::get(":/ui/emoteButton/emoteButton.css"));

    callButton->setObjectName("green");
    callButton->setStyleSheet(Style::get(":/ui/callButton/callButton.css"));

    videoButton->setObjectName("green");
    videoButton->setStyleSheet(Style::get(":/ui/videoButton/videoButton.css"));

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

    //Fix for incorrect layouts on OS X as per
    //https://bugreports.qt-project.org/browse/QTBUG-14591
    sendButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    fileButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    emoteButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    //    callButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    //    videoButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    //    msgEdit->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    //    chatArea->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    connect(Widget::getInstance()->getCore(), &Core::fileSendStarted, this, &ChatForm::startFileSend);
    connect(Widget::getInstance()->getCore(), &Core::videoFrameReceived, netcam, &NetCamView::updateDisplay);
    connect(sendButton, &QPushButton::clicked, this, &ChatForm::onSendTriggered);
    connect(fileButton, &QPushButton::clicked, this, &ChatForm::onAttachClicked);
    connect(callButton, &QPushButton::clicked, this, &ChatForm::onCallTriggered);
    connect(videoButton, &QPushButton::clicked, this, &ChatForm::onVideoCallTriggered);
    connect(msgEdit, &ChatTextEdit::enterPressed, this, &ChatForm::onSendTriggered);
    connect(chatArea->verticalScrollBar(), &QScrollBar::rangeChanged, this, &ChatForm::onSliderRangeChanged);
    connect(chatArea, &QScrollArea::customContextMenuRequested, this, &ChatForm::onChatContextMenuRequested);
    connect(emoteButton,  &QPushButton::clicked, this, &ChatForm::onEmoteButtonClicked);
}

ChatForm::~ChatForm()
{
    delete main;
    delete head;
    delete netcam;
}

void ChatForm::show(Ui::MainWindow &ui)
{
    ui.mainContent->layout()->addWidget(main);
    ui.mainHead->layout()->addWidget(head);
    main->show();
    head->show();
}

void ChatForm::setName(QString newName)
{
    name->setText(newName);
    name->setToolTip(newName); // for overlength names
}

void ChatForm::setStatusMessage(QString newMessage)
{
    statusMessage->setText(newMessage);
    statusMessage->setToolTip(newMessage); // for overlength messsages
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
    message = SmileyPack::getInstance().smileyfied(message);
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
        pal.setColor(QPalette::WindowText, QColor(100,100,100));
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

    QColor greentext(61,204,61);
    QString fontTemplate = "<font color='%1'>%2</font>";

    QString finalMessage;
    QStringList messageLines = message->text().split("\n");
    for (QString& s : messageLines)
    {
        if (QRegExp("^[ ]*>.*").exactMatch(s))
            finalMessage += fontTemplate.arg(greentext.name(), s.replace(" ", "&nbsp;"));
        else
            finalMessage += s.replace(" ", "&nbsp;");
        finalMessage += "<br>";
    }
    message->setText(finalMessage.left(finalMessage.length()-4));
    message->setTextFormat(Qt::RichText);

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
    QString path = QFileDialog::getOpenFileName(0,tr("Send a file"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return;
    long long filesize = file.size();
    file.close();
    QFileInfo fi(path);

    emit sendFile(f->friendId, fi.fileName(), path, filesize);
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

void ChatForm::onAvInvite(int FriendId, int CallId, bool video)
{
    if (FriendId != f->friendId)
        return;
    callId = CallId;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("yellow");
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onAnswerCallTriggered()));
    }
    else
    {
        callButton->setObjectName("yellow");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onAnswerCallTriggered()));
    }

    Widget* w = Widget::getInstance();
    if (!w->isFriendWidgetCurActiveWidget(f))
    {
        w->newMessageAlert();
        f->hasNewMessages=true;
        w->updateFriendStatusLights(f->friendId);
    }
}

void ChatForm::onAvStart(int FriendId, int CallId, bool video)
{
    if (FriendId != f->friendId)
        return;
    callId = CallId;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("red");
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
        netcam->show();
    }
    else
    {
        callButton->setObjectName("red");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
    }
}

void ChatForm::onAvCancel(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvEnd(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvRinging(int FriendId, int CallId, bool video)
{
    if (FriendId != f->friendId)
        return;
    callId = CallId;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("yellow");
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onCancelCallTriggered()));
    }
    else
    {
        callButton->setObjectName("yellow");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onCancelCallTriggered()));
    }
}

void ChatForm::onAvStarting(int FriendId, int, bool video)
{
    if (FriendId != f->friendId)
        return;
    callButton->disconnect();
    videoButton->disconnect();
    if (video)
    {
        callButton->setObjectName("grey");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("red");
        videoButton->style()->polish(videoButton);
        connect(videoButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
        netcam->show();
    }
    else
    {
        callButton->setObjectName("red");
        callButton->style()->polish(callButton);
        videoButton->setObjectName("grey");
        videoButton->style()->polish(videoButton);
        connect(callButton, SIGNAL(clicked()), this, SLOT(onHangupCallTriggered()));
    }
}

void ChatForm::onAvEnding(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvRequestTimeout(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
}

void ChatForm::onAvPeerTimeout(int FriendId, int)
{
    if (FriendId != f->friendId)
        return;
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
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
    videoButton->disconnect();
    emit startCall(f->friendId);
}

void ChatForm::onVideoCallTriggered()
{
    callButton->disconnect();
    videoButton->disconnect();
    emit startVideoCall(f->friendId, true);
}

void ChatForm::onCancelCallTriggered()
{
    callButton->disconnect();
    videoButton->disconnect();
    callButton->setObjectName("green");
    callButton->style()->polish(callButton);
    callButton->disconnect();
    videoButton->setObjectName("green");
    videoButton->style()->polish(videoButton);
    videoButton->disconnect();
    connect(callButton, SIGNAL(clicked()), this, SLOT(onCallTriggered()));
    connect(videoButton, SIGNAL(clicked()), this, SLOT(onVideoCallTriggered()));
    netcam->hide();
    emit cancelCall(callId, f->friendId);
}

void ChatForm::onChatContextMenuRequested(QPoint pos)
{
    QWidget* sender = (QWidget*)QObject::sender();
    pos = sender->mapToGlobal(pos);
    QMenu menu;
    menu.addAction(tr("Save chat log"), this, SLOT(onSaveLogClicked()));
    menu.exec(pos);
}

void ChatForm::onSaveLogClicked()
{
    QString path = QFileDialog::getSaveFileName(0,tr("Save chat log"));
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

void ChatForm::onEmoteButtonClicked()
{
    // don't show the smiley selection widget if there are no smileys available
    if (SmileyPack::getInstance().getEmoticons().empty())
        return;

    EmoticonsWidget widget;
    connect(&widget, &EmoticonsWidget::insertEmoticon, this, &ChatForm::onEmoteInsertRequested);

    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender)
    {
        QPoint pos = -QPoint(widget.sizeHint().width() / 2, widget.sizeHint().height()) - QPoint(0, 10);
        widget.exec(sender->mapToGlobal(pos));
    }
}

void ChatForm::onEmoteInsertRequested(QString str)
{
    // insert the emoticon
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender)
        msgEdit->insertPlainText(str);

    msgEdit->setFocus(); // refocus so that we can continue typing
}
