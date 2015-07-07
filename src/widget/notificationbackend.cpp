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

#include "notificationbackend.h"
#include <QApplication>
#include <QComboBox>
#include <QPointer>

NotificationBackend::NotificationBackend(QObject *parent)
    : QObject(parent)
{
    snoreCore.loadPlugins(Snore::SnorePlugin::BACKEND);
    snoreApp = Snore::Application(QApplication::applicationName(), Snore::Icon());
    alert[0] = Snore::Alert(tr("New Message"), Snore::Icon());
    alert[1] = Snore::Alert(tr("Highlighted"), Snore::Icon());
    alert[2] = Snore::Alert(tr("File Transfer Finished"), Snore::Icon());
    alert[3] = Snore::Alert(tr("Friend Request"), Snore::Icon());
    alert[4] = Snore::Alert(tr("AV Call"), Snore::Icon());
    alert[5] = Snore::Alert(tr("Friend Online"), Snore::Icon());

    snoreApp.hints().setValue("windows-app-id", "ToxFoundation.qTox");
    snoreApp.hints().setValue("desktop-entry", QApplication::applicationName());

    snoreCore.registerApplication(snoreApp);
    snoreCore.setPrimaryNotificationBackend();
}

void NotificationBackend::notify(Type type, GenericChatroomWidget *chat, const QString &title, const QString &message)
{
    Snore::Notification notification(snoreApp, alert[type], title, message, Snore::Icon());
    connect(&snoreCore, &Snore::SnoreCore::actionInvoked, this, &NotificationBackend::notificationInvoked);
    connect(&snoreCore, &Snore::SnoreCore::notificationClosed, this, &NotificationBackend::notificationClose);

    snoreCore.broadcastNotification(notification);

    chatList[notification.id()] = chat;
}

QWidget* NotificationBackend::settingsWidget()
{
    QComboBox* comboBox = new QComboBox();

    for (QString backend : snoreCore.notificationBackends())
        comboBox->addItem(backend);

    connect(comboBox, SIGNAL(activated(QString)), this, SLOT(setOptions(QString)));
    connect(this, &NotificationBackend::optionChanged, comboBox, &QComboBox::setCurrentText);

    return comboBox;
}

void NotificationBackend::setOptions(const QString& option)
{
    if (!snoreCore.setPrimaryNotificationBackend(option))
    {
        snoreCore.setPrimaryNotificationBackend();
        emit optionChanged(snoreCore.primaryNotificationBackend());
    }

}

void NotificationBackend::notificationInvoked(Snore::Notification notification)
{
    emit activated(chatList[notification.id()]);
}

void NotificationBackend::notificationClose(Snore::Notification notification)
{
    chatList.remove(notification.id());
}
