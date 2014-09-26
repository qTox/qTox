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

#include <QLabel>

#include "genericchatroomwidget.h"
#include "croppinglabel.h"

class QPixmap;

struct FriendWidget : public GenericChatroomWidget
{
    Q_OBJECT
public:
    FriendWidget(int FriendId, QString id);
    void contextMenuEvent(QContextMenuEvent * event);
    void setAsActiveChatroom();
    void setAsInactiveChatroom();
    void updateStatusLight();
    void setChatForm(Ui::MainWindow &);
    void resetEventFlags();

signals:
    void friendWidgetClicked(FriendWidget* widget);
    void removeFriend(int friendId);
    void copyFriendIdToClipboard(int friendId);

public slots:
    void onAvatarChange(int FriendId, const QPixmap& pic);
    void onAvatarRemoved(int FriendId);

protected:
    void mousePressEvent(QMouseEvent* ev);
    void mouseMoveEvent(QMouseEvent* ev);

public:
    int friendId;
    QLabel avatar, statusPic;
    CroppingLabel name, statusMessage;
    bool isDefaultAvatar;
    QPoint dragStartPos;
};

#endif // FRIENDWIDGET_H
