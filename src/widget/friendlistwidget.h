/*
    Copyright © 2014-2015 by The qTox Project

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FRIENDLISTWIDGET_H
#define FRIENDLISTWIDGET_H

#include <QWidget>
#include "src/core/corestructs.h"
#include "genericchatitemlayout.h"

class QVBoxLayout;
class QGridLayout;
class QPixmap;
class Widget;
class FriendWidget;
class GroupWidget;
class CircleWidget;
class FriendListLayout;
class GenericChatroomWidget;

class FriendListWidget : public QWidget
{
    Q_OBJECT
public:
    enum Mode : uint8_t
    {
        Name,
        Activity,
    };

    explicit FriendListWidget(Widget* parent, bool groupsOnTop = true);
    ~FriendListWidget();
    void setMode(Mode mode);
    Mode getMode() const;

    void addGroupWidget(GroupWidget* widget);
    void addFriendWidget(FriendWidget* w, Status s, int circleIndex);
    void removeFriendWidget(FriendWidget* w);
    void addCircleWidget(int id);
    void addCircleWidget(FriendWidget* widget = nullptr);
    void removeCircleWidget(CircleWidget* widget);
    void searchChatrooms(const QString &searchString, bool hideOnline = false, bool hideOffline = false, bool hideGroups = false);

    void cycleContacts(GenericChatroomWidget* activeChatroomWidget, bool forward);
    QVector<CircleWidget*> getAllCircles();

    void reDraw();

signals:
    void onCompactChanged(bool compact);

public slots:
    void renameGroupWidget(GroupWidget* groupWidget, const QString& newName);
    void renameCircleWidget(CircleWidget* circleWidget, const QString& newName);
    void onGroupchatPositionChanged(bool top);
    void moveWidget(FriendWidget* w, Status s, bool add = false);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    CircleWidget* createCircleWidget(int id = -1);
    QLayout* nextLayout(QLayout* layout, bool forward) const;

    Mode mode;
    bool groupsOnTop;
    FriendListLayout* listLayout;
    GenericChatItemLayout* circleLayout = nullptr;
    GenericChatItemLayout groupLayout;
    QVBoxLayout* activityLayout = nullptr;
};

#endif // FRIENDLISTWIDGET_H
