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

#ifndef FRIENDWIDGET_H
#define FRIENDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

struct FriendWidget : public QWidget
{
    Q_OBJECT
public:
    FriendWidget(int FriendId, QString id);
    void mouseReleaseEvent (QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent * event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void setAsActiveChatroom();
    void setAsInactiveChatroom();
    void setNewFixedWidth(int newWidth);

signals:
    void friendWidgetClicked(FriendWidget* widget);
    void removeFriend(int friendId);
    void copyFriendIdToClipboard(int friendId);

public:
    int friendId;
    QLabel avatar, name, statusMessage, statusPic;
    QHBoxLayout layout;
    QVBoxLayout textLayout;

private:
    QColor lastColor;
    int isActiveWidget;
};

#endif // FRIENDWIDGET_H
