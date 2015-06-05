/*
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

#ifndef CIRCLEWIDGET_H
#define CIRCLEWIDGET_H

#include "genericchatitemwidget.h"
#include "src/core/corestructs.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class FriendListWidget;
class FriendListLayout;
class FriendWidget;

class CircleWidget : public GenericChatItemWidget
{
    Q_OBJECT
public:
    CircleWidget(FriendListWidget* parent = 0, int id = -1);

    void addFriendWidget(FriendWidget* w, Status s);

    void searchChatrooms(const QString &searchString, bool hideOnline = false, bool hideOffline = false);

    void expand();
    void setExpanded(bool isExpanded);

    void updateStatus();

    void setName(const QString &name);
    void renameCircle();

    bool cycleContacts(bool forward);
    bool cycleContacts(FriendWidget* activeChatroomWidget, bool forward);

    bool hasChatrooms() const;

    static CircleWidget* getFromID(int id);

signals:
    void renameRequested(const QString &newName);

public slots:
    void onCompactChanged(bool compact);

protected:

    void contextMenuEvent(QContextMenuEvent* event);

    void mousePressEvent(QMouseEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void updateID(int index);
    static QHash<int, CircleWidget*> circleList;
    int id;
    bool expanded = false;
    FriendListLayout* listLayout;
    QVBoxLayout* fullLayout;
    QVBoxLayout* mainLayout = nullptr;
    QLabel* arrowLabel;
    QLabel* statusLabel;
    QFrame* lineFrame;
    QWidget* container;
    QHBoxLayout* topLayout = nullptr;
    QWidget* listWidget;
};

#endif // CIRCLEWIDGET_H
