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
class QLineEdit;

class CircleWidget : public GenericChatItemWidget
{
    Q_OBJECT
public:
    CircleWidget(FriendListWidget *parent = 0);

    void addFriendWidget(FriendWidget *w, Status s);

    void searchChatrooms(const QString &searchString, bool hideOnline = false, bool hideOffline = false, bool hideGroups = false);

    void expand();
    void toggle();

    void updateStatus();

    QString getName() const;
    void setName(const QString &name);
    void renameCircle();

public slots:
    void onCompactChanged(bool compact);

protected:

    void contextMenuEvent(QContextMenuEvent *event);

    void mousePressEvent(QMouseEvent *event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent* event) override;

private:
    enum FriendLayoutType
    {
        Online = 0,
        Offline = 1
    };
    bool expanded = false;
    FriendListLayout* listLayout;
    QVBoxLayout* fullLayout;
    QVBoxLayout* mainLayout = nullptr;
    QLabel* arrowLabel;
    QLabel* nameLabel;
    QLabel* statusLabel;
    QFrame* lineFrame;
    QWidget* container;
    QHBoxLayout* topLayout = nullptr;
};

#endif // CIRCLEWIDGET_H
