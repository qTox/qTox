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

#include "groupchatform.h"
#include "group.h"
#include "widget/groupwidget.h"
#include "widget/widget.h"
#include "friend.h"
#include "friendlist.h"
#include <QFont>
#include <QTime>
#include <QScrollBar>
#include <QMenu>
#include <QFile>
#include <QFileDialog>

GroupChatForm::GroupChatForm(Group* chatGroup)
    : group(chatGroup), curRow{0}, lockSliderToBottom{true}
{
    main = new QWidget(), head = new QWidget(), chatAreaWidget = new QWidget();
    headLayout = new QHBoxLayout(), mainFootLayout = new QHBoxLayout();
    headTextLayout = new QVBoxLayout(), mainLayout = new QVBoxLayout();
    mainChatLayout = new QGridLayout();
    avatar = new QLabel(), name = new QLabel(), nusers = new QLabel(), namesList = new QLabel();
    msgEdit = new ChatTextEdit();
    sendButton = new QPushButton();
    chatArea = new QScrollArea();
    QFont bold;
    bold.setBold(true);
    QFont small;
    small.setPixelSize(10);
    name->setText(group->widget->name.text());
    name->setFont(bold);
    nusers->setFont(small);
    nusers->setText(GroupChatForm::tr("%1 users in chat","Number of users in chat").arg(group->peers.size()));
    avatar->setPixmap(QPixmap(":/img/group.png"));
    QString names;
    for (QString& s : group->peers)
        names.append(s+", ");
    names.chop(2);
    namesList->setText(names);
    namesList->setFont(small);

    chatAreaWidget->setLayout(mainChatLayout);
    QString chatAreaStylesheet = "";
    try
    {
        QFile f(":/ui/chatArea/chatArea.css");
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


    QString msgEditStylesheet = "";
    try
    {
        QFile f(":/ui/msgEdit/msgEdit.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream msgEditStylesheetStream(&f);
        msgEditStylesheet = msgEditStylesheetStream.readAll();
    }
    catch (int e) {}
    msgEdit->setObjectName("group");
    msgEdit->setStyleSheet(msgEditStylesheet);
    msgEdit->setFixedHeight(50);
    msgEdit->setFrameStyle(QFrame::NoFrame);

    mainChatLayout->setColumnStretch(1,1);
    mainChatLayout->setHorizontalSpacing(10);

    QString sendButtonStylesheet = "";
    try
    {
        QFile f(":/ui/sendButton/sendButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream sendButtonStylesheetStream(&f);
        sendButtonStylesheet = sendButtonStylesheetStream.readAll();
    }
    catch (int e) {}
    sendButton->setStyleSheet(sendButtonStylesheet);

    sendButton->setFixedSize(50, 50);

    main->setLayout(mainLayout);
    mainLayout->addWidget(chatArea);
    mainLayout->addLayout(mainFootLayout);
    mainLayout->setMargin(0);

    mainFootLayout->addWidget(msgEdit);
    mainFootLayout->addWidget(sendButton);

    head->setLayout(headLayout);
    headLayout->addWidget(avatar);
    headLayout->addLayout(headTextLayout);
    headLayout->addStretch();
    headLayout->setMargin(0);

    headTextLayout->addStretch();
    headTextLayout->addWidget(name);
    headTextLayout->addWidget(nusers);
    headTextLayout->addWidget(namesList);
    headTextLayout->setMargin(0);
    headTextLayout->setSpacing(0);
    headTextLayout->addStretch();

    chatArea->setWidget(chatAreaWidget);

    sendButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendTriggered()));
    connect(msgEdit, SIGNAL(enterPressed()), this, SLOT(onSendTriggered()));
    connect(chatArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(onSliderRangeChanged()));
    connect(chatArea, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
}

GroupChatForm::~GroupChatForm()
{
    delete head;
    delete main;
}

void GroupChatForm::show(Ui::Widget &ui)
{
    ui.mainContent->layout()->addWidget(main);
    ui.mainHead->layout()->addWidget(head);
    main->show();
    head->show();
}

void GroupChatForm::setName(QString newName)
{
    name->setText(newName);
}

void GroupChatForm::onSendTriggered()
{
    QString msg = msgEdit->toPlainText();
    if (msg.isEmpty())
        return;
    msgEdit->clear();
    emit sendMessage(group->groupId, msg);
}

void GroupChatForm::addGroupMessage(QString message, int peerId)
{
    QLabel *msgAuthor;
    if (group->peers.contains(peerId))
        msgAuthor = new QLabel(group->peers[peerId]);
    else
        msgAuthor = new QLabel(tr("<Unknown>"));

    QLabel *msgText = new QLabel(message);
    QLabel *msgDate = new QLabel(QTime::currentTime().toString("hh:mm"));

    addMessage(msgAuthor, msgText, msgDate);
}

void GroupChatForm::addMessage(QString author, QString message, QString date)
{
    addMessage(new QLabel(author), new QLabel(message), new QLabel(date));
}

void GroupChatForm::addMessage(QLabel* author, QLabel* message, QLabel* date)
{
    QPalette greentext;
    greentext.setColor(QPalette::WindowText, QColor(61,204,61));
    QScrollBar* scroll = chatArea->verticalScrollBar();
    lockSliderToBottom = scroll && scroll->value() == scroll->maximum();
    author->setAlignment(Qt::AlignTop | Qt::AlignLeft);
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
    if (message->text()[0] == '>')
        message->setPalette(greentext);
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

void GroupChatForm::onSliderRangeChanged()
{
    QScrollBar* scroll = chatArea->verticalScrollBar();
    if (lockSliderToBottom)
         scroll->setValue(scroll->maximum());
}

void GroupChatForm::onUserListChanged()
{
    nusers->setText(tr("%1 users in chat").arg(group->nPeers));
    QString names;
    for (QString& s : group->peers)
        names.append(s+", ");
    names.chop(2);
    namesList->setText(names);
}

void GroupChatForm::onChatContextMenuRequested(QPoint pos)
{
    QWidget* sender = (QWidget*)QObject::sender();
    pos = sender->mapToGlobal(pos);
    QMenu menu;
    menu.addAction("Save chat log", this, SLOT(onSaveLogClicked()));
    menu.exec(pos);
}

void GroupChatForm::onSaveLogClicked()
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
