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

#include "snorenotificationbackend.h"
#include <QApplication>
#include <QPushButton>
#include <QBoxLayout>
#include <libsnore/settingsdialog.h>

SnoreNotificationBackend::SnoreNotificationBackend(QObject *parent)
    : NotificationBackend(parent)
{
    Snore::Icon icon(QIcon("://img/icons/qtox.svg").pixmap(48, 48).toImage());
    snoreApp = Snore::Application(QApplication::applicationName(), icon);
    snoreApp.addAlert(Snore::Alert(typeToString(NewMessage), icon));
    snoreApp.addAlert(Snore::Alert(typeToString(Highlighted), icon));
    snoreApp.addAlert(Snore::Alert(typeToString(FileTransferFinished), icon));
    snoreApp.addAlert(Snore::Alert(typeToString(FriendRequest), icon));
    snoreApp.addAlert(Snore::Alert(typeToString(AVCall), icon));

    snoreApp.hints().setValue("windows-app-id", "ToxFoundation.qTox");
    snoreApp.hints().setValue("desktop-entry", QApplication::applicationName());

    Snore::SnoreCore::instance().loadPlugins(Snore::SnorePlugin::BACKEND);
    Snore::SnoreCore::instance().registerApplication(snoreApp);
}

void SnoreNotificationBackend::notify(Type type, GenericChatroomWidget *chat, const QString &title, const QString &message, const QPixmap &icon)
{
    Snore::Icon snoreIcon(icon.toImage());
    Snore::Notification notification(snoreApp, snoreApp.alerts()[typeToString(type)], title, message, snoreIcon);
    connect(&Snore::SnoreCore::instance(), &Snore::SnoreCore::actionInvoked, this, &SnoreNotificationBackend::notificationInvoked);
    connect(&Snore::SnoreCore::instance(), &Snore::SnoreCore::notificationClosed, this, &SnoreNotificationBackend::notificationClose);

    Snore::SnoreCore::instance().broadcastNotification(notification);

    chatList[notification.id()] = chat;
}

QWidget* SnoreNotificationBackend::settingsWidget()
{
    QWidget* settings = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(settings);
    Snore::SettingsDialog* dialog = new Snore::SettingsDialog(settings);
    dialog->layout()->setMargin(0);
    QPushButton* apply = new QPushButton(tr("Apply"), settings);

    connect(apply, &QPushButton::clicked, [dialog]()
    {
        dialog->accept();
    });

    layout->addWidget(dialog);

    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->addStretch(1);
    hlayout->addWidget(apply);
    layout->addLayout(hlayout);

    return settings;
}

void SnoreNotificationBackend::setOptions(const QString& option)
{
    Snore::SnoreCore::instance().setPrimaryNotificationBackend(option);
}

void SnoreNotificationBackend::notificationInvoked(Snore::Notification notification)
{
    emit activated(chatList[notification.id()]);
}

void SnoreNotificationBackend::notificationClose(Snore::Notification notification)
{
    chatList.remove(notification.id());
}

QString SnoreNotificationBackend::typeToString(Type type)
{
    switch (type)
    {
        case NewMessage:
            return QStringLiteral("New Message");
        case Highlighted:
            return QStringLiteral("Highlighted");
        case FileTransferFinished:
            return QStringLiteral("File Transfer Finished");
        case FriendRequest:
            return QStringLiteral("Friend Request");
        case AVCall:
            return QStringLiteral("AV Call");
        default:
            return QString();
    }
}
