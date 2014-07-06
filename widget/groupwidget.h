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

#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

class GroupWidget : public QWidget
{
    Q_OBJECT
public:
    GroupWidget(int GroupId, QString Name);
    void onUserListChanged();
    void mouseReleaseEvent (QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent * event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);


signals:
    void groupWidgetClicked(GroupWidget* widget);
    void removeGroup(int groupId);

public:
    int groupId;
    QLabel avatar, name, nusers, statusPic;
    QHBoxLayout layout;
    QVBoxLayout textLayout;
    void setAsInactiveChatroom();
    void setAsActiveChatroom();
    void setNewFixedWidth(int newWidth);

private:
    QColor lastColor;
    int isActiveWidget;
};

#endif // GROUPWIDGET_H
