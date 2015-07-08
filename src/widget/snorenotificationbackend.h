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

#ifndef SNORENOTIFICATIONBACKEND_H
#define SNORENOTIFICATIONBACKEND_H

#include "notificationbackend.h"
#include <libsnore/snore.h>

class SnoreNotificationBackend : public NotificationBackend
{
    Q_OBJECT
public:
    SnoreNotificationBackend(QObject* parent = 0);
    virtual void notify(Type type, GenericChatroomWidget* chat, const QString& title, const QString& message, const QPixmap &icon) final override;
    virtual QWidget* settingsWidget() final override;

private slots:
    void setOptions(const QString& option);
    void notificationInvoked(Snore::Notification notification);
    void notificationClose(Snore::Notification notification);

private:
    QString typeToString(Type type);

    Snore::Application snoreApp;
    QHash<uint, GenericChatroomWidget*> chatList;
};

#endif // SNORENOTIFICATIONBACKEND_H
