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

#if DESKTOP_NOTIFICATIONS
#include <libsnore/snore.h>

#include <QObject>
#include <memory>

class DesktopNotify : public QObject
{
    Q_OBJECT
public:
    DesktopNotify();

    enum class MessageType {
        FRIEND,
        FRIEND_FILE,
        FRIEND_REQUEST,
        GROUP,
        GROUP_INVITE
    };

public slots:
    void notifyMessage(const QString& title, const QString& message);
    void notifyMessagePixmap(const QString& title, const QString& message, QPixmap avatar);
    void notifyMessageSimple(const MessageType type);

private:
    void createNotification(const QString& title, const QString& text, Snore::Icon& icon);

private:
    Snore::SnoreCore& notifyCore;
    Snore::Application snoreApp;
    Snore::Icon snoreIcon;
};
#endif // DESKTOP_NOTIFICATIONS
