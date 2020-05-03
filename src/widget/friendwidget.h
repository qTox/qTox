/*
    Copyright © 2019 by The qTox Project Contributors

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

#pragma once

#include "genericchatroomwidget.h"
#include "src/core/toxpk.h"

#include <memory>

class FriendChatroom;
class QPixmap;
class MaskablePixmapWidget;
class CircleWidget;

class FriendWidget : public GenericChatroomWidget
{
    Q_OBJECT
public:
    FriendWidget(std::shared_ptr<FriendChatroom> chatform, bool compact);

    void contextMenuEvent(QContextMenuEvent* event) final;
    void setAsActiveChatroom() final;
    void setAsInactiveChatroom() final;
    void updateStatusLight() final;
    void resetEventFlags() final;
    QString getStatusString() const final;
    const Friend* getFriend() const final;
    const Contact* getContact() const final;

    void search(const QString& searchString, bool hide = false);

signals:
    void friendWidgetClicked(FriendWidget* widget);
    void removeFriend(const ToxPk& friendPk);
    void copyFriendIdToClipboard(const ToxPk& friendPk);
    void contextMenuCalled(QContextMenuEvent* event);
    void friendHistoryRemoved();
    void friendWidgetRenamed(FriendWidget* friendWidget);
    void searchCircle(CircleWidget& circleWidget);
    void updateFriendActivity(Friend& frnd);

public slots:
    void onAvatarSet(const ToxPk& friendPk, const QPixmap& pic);
    void onAvatarRemoved(const ToxPk& friendPk);
    void onContextMenuCalled(QContextMenuEvent* event);
    void setActive(bool active);

protected:
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void setFriendAlias();

private slots:
    void removeChatWindow();
    void moveToNewCircle();
    void removeFromCircle();
    void moveToCircle(int circleId);
    void changeAutoAccept(bool enable);
    void showDetails();

public:
    std::shared_ptr<FriendChatroom> chatroom;
    bool isDefaultAvatar;
};
