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

#include "genericchatform.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include "smileypack.h"
#include "widget/emoticonswidget.h"
#include "style.h"
#include "widget/widget.h"
#include "settings.h"
#include "widget/tool/chataction.h"

GenericChatForm::GenericChatForm(QObject *parent) :
    QObject(parent)
{
    curRow = 0;

    mainWidget = new QWidget(); headWidget = new QWidget();

    nameLabel = new CroppingLabel();
    avatarLabel = new QLabel();
    QHBoxLayout *headLayout = new QHBoxLayout(), *mainFootLayout = new QHBoxLayout();
    headTextLayout = new QVBoxLayout();
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QVBoxLayout *footButtonsSmall = new QVBoxLayout(), *volMicLayout = new QVBoxLayout();

    chatWidget = new ChatAreaWidget();
    chatWidget->document()->setDefaultStyleSheet(Style::get(":ui/chatArea/innerStyle.css"));
    chatWidget->setStyleSheet(Style::get(":/ui/chatArea/chatArea.css"));

    msgEdit = new ChatTextEdit();

    sendButton = new QPushButton();
    emoteButton = new QPushButton();

    fileButton = new QPushButton();
    callButton = new QPushButton();
    videoButton = new QPushButton();
    volButton = new QPushButton();
    micButton = new QPushButton();

    QFont bold;
    bold.setBold(true);
    nameLabel->setFont(bold);

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

    QString volButtonStylesheet = Style::get(":/ui/volButton/volButton.css");
    volButton->setObjectName("green");
    volButton->setStyleSheet(volButtonStylesheet);

    QString micButtonStylesheet = Style::get(":/ui/micButton/micButton.css");
    micButton->setObjectName("green");
    micButton->setStyleSheet(micButtonStylesheet);

    mainWidget->setLayout(mainLayout);
    mainLayout->addWidget(chatWidget);
    mainLayout->addLayout(mainFootLayout);
    mainLayout->setMargin(0);

    footButtonsSmall->addWidget(emoteButton);
    footButtonsSmall->addWidget(fileButton);

    mainFootLayout->addWidget(msgEdit);
    mainFootLayout->addLayout(footButtonsSmall);
    mainFootLayout->addSpacing(5);
    mainFootLayout->addWidget(sendButton);
    mainFootLayout->setSpacing(0);

    headWidget->setLayout(headLayout);
    headLayout->addWidget(avatarLabel);
    headLayout->addLayout(headTextLayout);
    headLayout->addLayout(volMicLayout);
    headLayout->addWidget(callButton);
    headLayout->addWidget(videoButton);

    volMicLayout->addWidget(micButton);
    volMicLayout->addWidget(volButton);

    headTextLayout->addStretch();
    headTextLayout->addWidget(nameLabel);

    //Fix for incorrect layouts on OS X as per
    //https://bugreports.qt-project.org/browse/QTBUG-14591
    sendButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    fileButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    emoteButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    connect(emoteButton,  SIGNAL(clicked()), this, SLOT(onEmoteButtonClicked()));
    connect(chatWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
}

void GenericChatForm::setName(const QString &newName)
{
    nameLabel->setText(newName);
    nameLabel->setToolTip(newName); // for overlength names
}

void GenericChatForm::show(Ui::MainWindow &ui)
{
    ui.mainContent->layout()->addWidget(mainWidget);
    ui.mainHead->layout()->addWidget(headWidget);
    mainWidget->show();
    headWidget->show();
}

void GenericChatForm::onChatContextMenuRequested(QPoint pos)
{
    QWidget* sender = (QWidget*)QObject::sender();
    pos = sender->mapToGlobal(pos);
    QMenu menu;
    menu.addAction(tr("Save chat log"), this, SLOT(onSaveLogClicked()));
    menu.exec(pos);
}

void GenericChatForm::onSaveLogClicked()
{
    QString path = QFileDialog::getSaveFileName(0, tr("Save chat log"));
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QString log;
    log = chatWidget->toPlainText();

    file.write(log.toUtf8());
    file.close();
}

void GenericChatForm::addMessage(QString author, QString message, QDateTime datetime)
{
    QString date = datetime.toString(Settings::getInstance().getTimestampFormat());
    bool isMe = (author == Widget::getInstance()->getUsername());

    if (previousName == author)
        chatWidget->insertMessage(new MessageAction("", message, date, isMe));
    else chatWidget->insertMessage(new MessageAction(author , message, date, isMe));
    previousName = author;
}

GenericChatForm::~GenericChatForm()
{
    delete mainWidget;
    delete headWidget;
}

void GenericChatForm::onEmoteButtonClicked()
{
    // don't show the smiley selection widget if there are no smileys available
    if (SmileyPack::getInstance().getEmoticons().empty())
        return;

    EmoticonsWidget widget;
    connect(&widget, SIGNAL(insertEmoticon(QString)), this, SLOT(onEmoteInsertRequested(QString)));

    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender)
    {
        QPoint pos = -QPoint(widget.sizeHint().width() / 2, widget.sizeHint().height()) - QPoint(0, 10);
        widget.exec(sender->mapToGlobal(pos));
    }
}

void GenericChatForm::onEmoteInsertRequested(QString str)
{
    // insert the emoticon
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender)
        msgEdit->insertPlainText(str);

    msgEdit->setFocus(); // refocus so that we can continue typing
}

void GenericChatForm::focusInput()
{
    msgEdit->setFocus();
}
