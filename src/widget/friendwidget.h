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

#include "genericchatroomwidget.h"

class QPixmap;
class MaskablePixmapWidget;

class FriendWidget : public GenericChatroomWidget
{
    Q_OBJECT
public:
    FriendWidget(int FriendId, QString id);
    virtual void contextMenuEvent(QContextMenuEvent * event) override;
    virtual void setAsActiveChatroom() override;
    virtual void setAsInactiveChatroom() override;
    virtual void updateStatusLight() override;
    virtual void setChatForm(Ui::MainWindow &) override;
    virtual void resetEventFlags() override;
    virtual QString getStatusString() override;

signals:
    void friendWidgetClicked(FriendWidget* widget);
    void removeFriend(int friendId);
    void copyFriendIdToClipboard(int friendId);

public slots:
    void onAvatarChange(int FriendId, const QPixmap& pic);
    void onAvatarRemoved(int FriendId);
    void setAlias(const QString& alias);

protected:
    virtual void mousePressEvent(QMouseEvent* ev) override;
    virtual void mouseMoveEvent(QMouseEvent* ev) override;
    void setFriendAlias();

public:
    int friendId;
    bool isDefaultAvatar;
    bool historyLoaded;
    QPoint dragStartPos;
};

#endif // FRIENDWIDGET_H
