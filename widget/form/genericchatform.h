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

#ifndef GENERICCHATFORM_H
#define GENERICCHATFORM_H

#include <QObject>
#include <QLabel>
#include <QPoint>
#include <QScrollArea>
#include <QTime>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGridLayout>

#include "widget/chatareawidget.h"
#include "widget/tool/chattextedit.h"
#include "widget/tool/genericchataction.h"

// Spacing in px inserted when the author of the last message changes
#define AUTHOR_CHANGE_SPACING 5

namespace Ui {
    class MainWindow;
}

class GenericChatForm : public QObject
{
    Q_OBJECT
public:
    GenericChatForm(QObject *parent = 0);
    virtual ~GenericChatForm();

    virtual void setName(const QString &newName);
    virtual void show(Ui::MainWindow &ui);
    void addMessage(QString author, QString message, QDateTime datetime=QDateTime::currentDateTime());

signals:
    void sendMessage(int, QString);

public slots:

protected slots:
    void onChatContextMenuRequested(QPoint pos);
    void onSliderRangeChanged();
    void onSaveLogClicked();
    void onEmoteButtonClicked();
    void onEmoteInsertRequested(QString str);
    void updateChatContent();

protected:
    QLabel *nameLabel, *avatarLabel;
    QWidget *mainWidget, *headWidget;
    QPushButton *fileButton, *emoteButton, *callButton, *videoButton, *volButton, *micButton;
    QVBoxLayout *headTextLayout;
    ChatTextEdit *msgEdit;
    QPushButton *sendButton;
    QString previousName;
    ChatAreaWidget *newChatForm;
    int curRow;
    bool lockSliderToBottom;

    QList<ChatAction*> messages;

private:
    QString getHtmledMessages();
};

#endif // GENERICCHATFORM_H
