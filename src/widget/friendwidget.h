/*
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

#ifndef FRIENDWIDGET_H
#define FRIENDWIDGET_H

#include "src/friend.h"
#include "src/widget/genericchatroomwidget.h"

class FriendListWidget;
class QPixmap;
class MaskablePixmapWidget;

class FriendWidget : public GenericChatroomWidget
{
    Q_OBJECT
public:
    FriendWidget(const Friend& _f, FriendListWidget* parent);

    Type type() const override final;
    void contextMenuEvent(QContextMenuEvent * event) override final;
    void setAsActiveChatroom(bool activate) override final;
    void updateStatusLight() override final;
    bool chatFormIsSet(bool focus) const override final;
    void resetEventFlags() override final;
    QString getTitle() const override final;

    void search(const QString &searchString, bool hide = false);

    void updateAvatar(Friend::ID FriendId, const QPixmap& pic);

    inline const Friend& getFriend() const
    {
        return f;
    }

signals:
    void friendWidgetClicked(FriendWidget* widget);
    void copyFriendIdToClipboard(uint32_t friendId);
    void contextMenuCalled(QContextMenuEvent * event);

public slots:
    void setAlias(const QString& alias);
    void onContextMenuCalled(QContextMenuEvent * event);

protected:
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void setFriendAlias();

private:
    Friend f;
    bool isDefaultAvatar;
    bool historyLoaded;
};

#endif // FRIENDWIDGET_H
