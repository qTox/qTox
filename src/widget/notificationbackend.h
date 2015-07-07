/*
    Copyright © 2015 by The qTox Project

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

#ifndef NOTIFICATIONBACKEND_H
#define NOTIFICATIONBACKEND_H

#include <QObject>
#include <snore/core/snore.h>

class GenericChatroomWidget;

class NotificationBackend : public QObject
{
    Q_OBJECT
public:
    enum Type
    {
        NewMessage = 0,
        Highlighted = 1,
        FileTransferFinished = 2,
        FriendRequest = 3,
        AVCall = 4,
        FriendStatusChanged = 5,
    };

    NotificationBackend(QObject* parent = 0);
    void notify(Type type, GenericChatroomWidget* chat, const QString& title, const QString& message);
    QWidget* settingsWidget();

signals:
    void activated(GenericChatroomWidget* chat);
    void optionChanged(const QString& option);

public slots:
    void setOptions(const QString& option);

private slots:
    void notificationInvoked(Snore::Notification notification);
    void notificationClose(Snore::Notification notification);

private:
    Snore::SnoreCore snoreCore;
    Snore::Application snoreApp;
    Snore::Alert alert[6];
    QHash<uint, GenericChatroomWidget*> chatList;
};

#endif // NOTIFICATIONBACKEND_H
