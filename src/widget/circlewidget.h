/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#ifndef CIRCLEWIDGET_H
#define CIRCLEWIDGET_H

#include "categorywidget.h"

class ContentDialog;

class CircleWidget final : public CategoryWidget
{
    Q_OBJECT
public:
    explicit CircleWidget(FriendListWidget* parent, int id);
    ~CircleWidget();

    void editName();
    static CircleWidget* getFromID(int id);

signals:
    void renameRequested(CircleWidget* circleWidget, const QString& newName);
    void searchCircle(CircleWidget& circletWidget);
    void newContentDialog(ContentDialog& contentDialog);

protected:
    void contextMenuEvent(QContextMenuEvent* event) final override;
    void dragEnterEvent(QDragEnterEvent* event) final override;
    void dragLeaveEvent(QDragLeaveEvent* event) final override;
    void dropEvent(QDropEvent* event) final override;

private:
    void onSetName() final override;
    void onExpand() final override;
    void onAddFriendWidget(FriendWidget* w) final override;
    void updateID(int index);

    static QHash<int, CircleWidget*> circleList;
    int id;
};

#endif // CIRCLEWIDGET_H
