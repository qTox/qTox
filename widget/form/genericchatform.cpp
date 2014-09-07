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
#include <QScrollBar>
#include <QFileDialog>
#include <QTextStream>
#include "smileypack.h"
#include "widget/emoticonswidget.h"
#include "style.h"
#include "widget/widget.h"

GenericChatForm::GenericChatForm(QObject *parent) :
    QObject(parent)
{
    lockSliderToBottom = true;
    curRow = 0;

    mainWidget = new QWidget(); headWidget = new QWidget(); chatAreaWidget = new QWidget();

    nameLabel = new QLabel();
    avatarLabel = new QLabel();
    QHBoxLayout *headLayout = new QHBoxLayout(), *mainFootLayout = new QHBoxLayout();
    headTextLayout = new QVBoxLayout();
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QVBoxLayout *footButtonsSmall = new QVBoxLayout(), *volMicLayout = new QVBoxLayout();
    mainChatLayout = new QGridLayout();

    msgEdit = new ChatTextEdit();

    sendButton = new QPushButton();
    emoteButton = new QPushButton();

    fileButton = new QPushButton();
    callButton = new QPushButton();
    videoButton = new QPushButton();
    volButton = new QPushButton();
    micButton = new QPushButton();

    chatArea = new QScrollArea();

    QFont bold;
    bold.setBold(true);
    nameLabel->setFont(bold);

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

    QString volButtonStylesheet = "";
    try
    {
        QFile f(":/ui/volButton/volButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream volButtonStylesheetStream(&f);
        volButtonStylesheet = volButtonStylesheetStream.readAll();
    }
    catch (int e) {}

    volButton->setObjectName("green");
    volButton->setStyleSheet(volButtonStylesheet);

    QString micButtonStylesheet = "";
    try
    {
        QFile f(":/ui/micButton/micButton.css");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream micButtonStylesheetStream(&f);
        micButtonStylesheet = micButtonStylesheetStream.readAll();
    }
    catch (int e) {}

    micButton->setObjectName("green");
    micButton->setStyleSheet(micButtonStylesheet);

    mainWidget->setLayout(mainLayout);
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

    headWidget->setLayout(headLayout);
    headLayout->addWidget(avatarLabel);
    headLayout->addLayout(headTextLayout);
    headLayout->addStretch();
    headLayout->addLayout(volMicLayout);
    headLayout->addWidget(callButton);
    headLayout->addWidget(videoButton);

    volMicLayout->addWidget(micButton);
    volMicLayout->addWidget(volButton);

    headTextLayout->addStretch();
    headTextLayout->addWidget(nameLabel);

    chatArea->setWidget(chatAreaWidget);

    //Fix for incorrect layouts on OS X as per
    //https://bugreports.qt-project.org/browse/QTBUG-14591
    sendButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    fileButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    emoteButton->setAttribute(Qt::WA_LayoutUsesWidgetRect);

    connect(emoteButton,  SIGNAL(clicked()), this, SLOT(onEmoteButtonClicked()));
    connect(chatArea, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
    connect(chatArea->verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(onSliderRangeChanged()));
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

void GenericChatForm::onSliderRangeChanged()
{
    QScrollBar* scroll = chatArea->verticalScrollBar();
    if (lockSliderToBottom)
         scroll->setValue(scroll->maximum());
}

void GenericChatForm::onSaveLogClicked()
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

void GenericChatForm::addMessage(QString author, QString message, QString date)
{
    QLabel *authorLabel = new QLabel(author);
    QLabel *messageLabel = new QLabel();
    QLabel *dateLabel = new QLabel(date);

    QScrollBar* scroll = chatArea->verticalScrollBar();
    lockSliderToBottom = scroll && scroll->value() == scroll->maximum();
    authorLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    dateLabel->setAlignment(Qt::AlignTop);
    messageLabel->setWordWrap(true);
    messageLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    authorLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    dateLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    if (author == Widget::getInstance()->getUsername())
    {
        QPalette pal;
        pal.setColor(QPalette::WindowText, QColor(100,100,100));
        authorLabel->setPalette(pal);
        messageLabel->setPalette(pal);
    }
    if (previousName.isEmpty() || previousName != author)
    {
        if (curRow)
        {
            mainChatLayout->setRowStretch(curRow, 0);
            mainChatLayout->addItem(new QSpacerItem(0,AUTHOR_CHANGE_SPACING),curRow,0,1,3);
        }
        previousName = author;
        curRow++;
    }
    else if (curRow)// onSaveLogClicked expects 0 or 3 QLabel per line
        authorLabel->setText("");

    QColor greentext(61,204,61);
    QString fontTemplate = "<font color='%1'>%2</font>";

    QString finalMessage;
    QStringList messageLines = message.split("\n");
    for (QString& s : messageLines)
    {
        if (QRegExp("^[ ]*>.*").exactMatch(s))
            finalMessage += fontTemplate.arg(greentext.name(), toHtmlChars(s));
        else
            finalMessage += toHtmlChars(s);
        finalMessage += "<br>";
    }
    messageLabel->setText(finalMessage.left(finalMessage.length()-4));
    messageLabel->setText(SmileyPack::getInstance().smileyfied(messageLabel->text()));
    messageLabel->setTextFormat(Qt::RichText);

    mainChatLayout->addWidget(authorLabel, curRow, 0);
    mainChatLayout->addWidget(messageLabel, curRow, 1);
    mainChatLayout->addWidget(dateLabel, curRow, 3);
    mainChatLayout->setRowStretch(curRow+1, 1);
    mainChatLayout->setRowStretch(curRow, 0);
    curRow++;
    authorLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    messageLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    dateLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(authorLabel, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
    connect(messageLabel, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
    connect(dateLabel, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onChatContextMenuRequested(QPoint)));
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

QString GenericChatForm::toHtmlChars(const QString &str)
{
    static QList<QPair<QString, QString>> replaceList = {{"&","&amp;"}, {" ","&nbsp;"}, {">","&gt;"}, {"<","&lt;"}};
    QString res = str;

    for (auto &it : replaceList)
        res = res.replace(it.first,it.second);

    return res;
}
