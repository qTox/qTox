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
#include "src/misc/smileypack.h"
#include "src/widget/emoticonswidget.h"
#include "src/misc/style.h"
#include "src/widget/widget.h"
#include "src/misc/settings.h"
#include "src/widget/tool/chatactions/messageaction.h"
#include "src/widget/tool/chatactions/systemmessageaction.h"
#include "src/widget/tool/chatactions/actionaction.h"
#include "src/widget/tool/chatactions/alertaction.h"
#include "src/widget/chatareawidget.h"
#include "src/widget/tool/chattextedit.h"
#include "src/widget/maskablepixmapwidget.h"

GenericChatForm::GenericChatForm(QWidget *parent) :
    QWidget(parent),
    earliestMessage(nullptr)
{
    curRow = 0;

    headWidget = new QWidget();

    nameLabel = new CroppingLabel();
    nameLabel->setObjectName("nameLabel");
    nameLabel->setMinimumHeight(Style::getFont(Style::Medium).pixelSize());

    avatar = new MaskablePixmapWidget(this, QSize(40,40), ":/img/avatar_mask.png");
    QHBoxLayout *headLayout = new QHBoxLayout(), *mainFootLayout = new QHBoxLayout();
    headTextLayout = new QVBoxLayout();
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QVBoxLayout *footButtonsSmall = new QVBoxLayout(), *volMicLayout = new QVBoxLayout();

    chatWidget = new ChatAreaWidget();

    msgEdit = new ChatTextEdit();

    sendButton = new QPushButton();
    sendButton->setToolTip(tr("Send message"));
    emoteButton = new QPushButton();
    emoteButton->setToolTip(tr("Smileys"));

    // Setting the sizes in the CSS doesn't work (glitch with high DPIs)
    fileButton = new QPushButton();
    fileButton->setToolTip(tr("Send a file"));
    callButton = new QPushButton();
    callButton->setFixedSize(50,40);
    callButton->setToolTip(tr("Audio call"));
    videoButton = new QPushButton();
    videoButton->setFixedSize(50,40);
    videoButton->setToolTip(tr("Video call"));
    volButton = new QPushButton();
    volButton->setFixedSize(25,20);
    volButton->setToolTip(tr("Toggle speakers volume"));
    micButton = new QPushButton();
    micButton->setFixedSize(25,20);
    micButton->setToolTip(tr("Toggle microphone"));

    footButtonsSmall->setSpacing(2);

    msgEdit->setStyleSheet(Style::getStylesheet(":/ui/msgEdit/msgEdit.css"));
    msgEdit->setFixedHeight(50);
    msgEdit->setFrameStyle(QFrame::NoFrame);

    sendButton->setStyleSheet(Style::getStylesheet(":/ui/sendButton/sendButton.css"));
    fileButton->setStyleSheet(Style::getStylesheet(":/ui/fileButton/fileButton.css"));
    emoteButton->setStyleSheet(Style::getStylesheet(":/ui/emoteButton/emoteButton.css"));

    callButton->setObjectName("green");
    callButton->setStyleSheet(Style::getStylesheet(":/ui/callButton/callButton.css"));

    videoButton->setObjectName("green");
    videoButton->setStyleSheet(Style::getStylesheet(":/ui/videoButton/videoButton.css"));

    QString volButtonStylesheet = Style::getStylesheet(":/ui/volButton/volButton.css");
    volButton->setObjectName("green");
    volButton->setStyleSheet(volButtonStylesheet);

    QString micButtonStylesheet = Style::getStylesheet(":/ui/micButton/micButton.css");
    micButton->setObjectName("green");
    micButton->setStyleSheet(micButtonStylesheet);

    setLayout(mainLayout);
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
    headLayout->addWidget(avatar);
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

    menu.addAction(tr("Save chat log"), this, SLOT(onSaveLogClicked()));
    menu.addAction(tr("Clear displayed messages"), this, SLOT(clearChatArea(bool)));
    menu.addSeparator();

    connect(emoteButton,  SIGNAL(clicked()), this, SLOT(onEmoteButtonClicked()));
    connect(chatWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));

    chatWidget->document()->setDefaultStyleSheet(Style::getStylesheet(":ui/chatArea/innerStyle.css"));
    chatWidget->setStyleSheet(Style::getStylesheet(":/ui/chatArea/chatArea.css"));
    headWidget->setStyleSheet(Style::getStylesheet(":/ui/chatArea/chatHead.css"));
}

int GenericChatForm::getNumberOfMessages()
{
    return chatWidget->getNumberOfMessages();
}

void GenericChatForm::setName(const QString &newName)
{
    nameLabel->setText(newName);
    nameLabel->setToolTip(newName); // for overlength names
}

void GenericChatForm::show(Ui::MainWindow &ui)
{
    ui.mainContent->layout()->addWidget(this);
    ui.mainHead->layout()->addWidget(headWidget);
    headWidget->show();
    QWidget::show();
}

void GenericChatForm::onChatContextMenuRequested(QPoint pos)
{
    QWidget* sender = (QWidget*)QObject::sender();
    pos = sender->mapToGlobal(pos);
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

void GenericChatForm::addMessage(const QString &author, const QString &message, bool isAction, const QDateTime &datetime)
{
    ChatAction *ca = genMessageActionAction(author, message, isAction, datetime);
    chatWidget->insertMessage(ca);
}

void GenericChatForm::addAlertMessage(QString author, QString message, QDateTime datetime)
{
    QString date = datetime.toString(Settings::getInstance().getTimestampFormat());
    chatWidget->insertMessage(new AlertAction(author, message, date));
    previousName = author;
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

void GenericChatForm::addSystemInfoMessage(const QString &message, const QString &type, const QDateTime &datetime)
{
    ChatAction *ca = genSystemInfoAction(message, type, datetime);
    chatWidget->insertMessage(ca);
}

QString GenericChatForm::getElidedName(const QString& name)
{
    // update this whenever you change the font in innerStyle.css
    QFontMetrics fm(Style::getFont(Style::BigBold));

    return fm.elidedText(name, Qt::ElideRight, chatWidget->nameColWidth());
}

void GenericChatForm::clearChatArea(bool notinform)
{
    chatWidget->clearChatArea();
    previousName = "";

    if (!notinform)
        addSystemInfoMessage(tr("Cleared"), "white", QDateTime::currentDateTime());

    if (earliestMessage)
    {
        delete earliestMessage;
        earliestMessage = nullptr;
    }
}

ChatAction* GenericChatForm::genMessageActionAction(const QString &author, QString message, bool isAction, const QDateTime &datetime)
{
    if (earliestMessage == nullptr)
    {
        earliestMessage = new QDateTime(datetime);
    }

    QString date = datetime.toString(Settings::getInstance().getTimestampFormat());
    bool isMe = (author == Widget::getInstance()->getUsername());

    if (!isAction && message.startsWith("/me "))
    { // always render actions regardless of what core thinks
        isAction = true;
        message = message.right(message.length()-4);
    }

    if (isAction)
    {
        previousName = ""; // next msg has a name regardless
        return (new ActionAction (getElidedName(author), message, date, isMe));
    }

    ChatAction *res;
    if (previousName == author)
        res = (new MessageAction("", message, date, isMe));
    else
        res = (new MessageAction(getElidedName(author), message, date, isMe));

    previousName = author;
    return res;
}

ChatAction* GenericChatForm::genSystemInfoAction(const QString &message, const QString &type, const QDateTime &datetime)
{
    previousName = "";
    QString date = datetime.toString(Settings::getInstance().getTimestampFormat());

    return (new SystemMessageAction(message, type, date));
}
