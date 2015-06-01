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
#include <QHash>
#include <QList>
#include <QVector>
#include "src/core/corestructs.h"
#include "src/widget/genericchatroomwidget.h"

#include "sortingboxlayout.h"

#include "circlewidget.h"

class QVBoxLayout;
class QGridLayout;
class QPixmap;

class FriendWidget;
class GroupWidget;
class CircleWidget;
class FriendListLayout;

class FriendListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FriendListWidget(QWidget *parent = 0, bool groupsOnTop = true);

    void addGroupWidget(GroupWidget *widget);
    void addFriendWidget(FriendWidget *w, Status s, int circleIndex);
    void addCircleWidget(const QString &name);
    CircleWidget *addCircleWidget(FriendWidget *widget = nullptr);
    void removeCircleWidget(CircleWidget *widget);

    void searchChatrooms(const QString &searchString, bool hideOnline = false, bool hideOffline = false, bool hideGroups = false);

    void cycleContacts(int index);
    QList<GenericChatroomWidget*> getAllFriends();
    QVector<CircleWidget*> getAllCircles();

    void reDraw();

public slots:
    void renameCircleWidget(const QString &newName);
    //void onCompactChanged(bool compact);
    void onGroupchatPositionChanged(bool top);
    void moveWidget(FriendWidget *w, Status s, bool add = false);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QVBoxLayout* getFriendLayout(Status s);
    enum FriendLayoutType
    {
        Online = 0,
        Offline = 1
    };
    FriendListLayout *listLayout;
    QVBoxLayout *circleLayout;
    VSortingBoxLayout<CircleWidget> circleLayout2;
};

#endif // FRIENDLISTWIDGET_H
