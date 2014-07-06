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

#ifndef GROUPCHATFORM_H
#define GROUPCHATFORM_H

#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTextEdit>
#include <QScrollArea>
#include <QTime>

#include "widget/tool/chattextedit.h"
#include "ui_widget.h"

// Spacing in px inserted when the author of the last message changes
#define AUTHOR_CHANGE_SPACING 5

class Group;

class GroupChatForm : public QObject
{
    Q_OBJECT
public:
    GroupChatForm(Group* chatGroup);
    ~GroupChatForm();
    void show(Ui::Widget& ui);
    void setName(QString newName);
    void addGroupMessage(QString message, int peerId);
    void addMessage(QString author, QString message, QString date=QTime::currentTime().toString("hh:mm"));
    void addMessage(QLabel* author, QLabel* message, QLabel* date);
    void onUserListChanged();

signals:
    void sendMessage(int, QString);

private slots:
    void onSendTriggered();
    void onSliderRangeChanged();
    void onChatContextMenuRequested(QPoint pos);
    void onSaveLogClicked();

private:
    Group* group;
    QHBoxLayout *headLayout, *mainFootLayout;
    QVBoxLayout *headTextLayout, *mainLayout;
    QGridLayout *mainChatLayout;
    QLabel *avatar, *name, *nusers, *namesList;
    ChatTextEdit *msgEdit;
    QPushButton *sendButton;
    QScrollArea *chatArea;
    QWidget *main, *head, *chatAreaWidget;
    QString previousName;
    int curRow;
    bool lockSliderToBottom;
};

#endif // GROUPCHATFORM_H
