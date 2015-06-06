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
#include "src/core/corestructs.h"
#include "src/widget/genericchatroomwidget.h"

class QVBoxLayout;
class QGridLayout;
class QPixmap;

class FriendListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FriendListWidget(QWidget *parent = 0, bool groupchatPosition = true);
    QVBoxLayout* getGroupLayout();
    QVBoxLayout* getFriendLayout(Status s);

    QList<GenericChatroomWidget*> getAllFriends();
    void reDraw();
signals:

public slots:
    void onGroupchatPositionChanged(bool top);
    void moveWidget(QWidget *w, Status s);

private:
    QHash<int, QVBoxLayout*> layouts;
    QVBoxLayout *groupLayout;
    QGridLayout *mainLayout;
};

#endif // FRIENDLISTWIDGET_H
