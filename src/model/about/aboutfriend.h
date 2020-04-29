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

#include "iaboutfriend.h"
#include "util/interface.h"
#include "src/persistence/ifriendsettings.h"

#include <QObject>

class Friend;
class IFriendSettings;

class AboutFriend : public QObject, public IAboutFriend
{
    Q_OBJECT

public:
    AboutFriend(const Friend* f, IFriendSettings* const settings);

    QString getName() const override;
    QString getStatusMessage() const override;
    ToxPk getPublicKey() const override;

    QPixmap getAvatar() const override;

    QString getNote() const override;
    void setNote(const QString& note) override;

    QString getAutoAcceptDir() const override;
    void setAutoAcceptDir(const QString& path) override;

    IFriendSettings::AutoAcceptCallFlags getAutoAcceptCall() const override;
    void setAutoAcceptCall(IFriendSettings::AutoAcceptCallFlags flag) override;

    bool getAutoGroupInvite() const override;
    void setAutoGroupInvite(bool enabled) override;

    bool clearHistory() override;
    bool isHistoryExistence() override;

    SIGNAL_IMPL(AboutFriend, nameChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, statusChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, publicKeyChanged, const QString&)

    SIGNAL_IMPL(AboutFriend, avatarChanged, const QPixmap&)
    SIGNAL_IMPL(AboutFriend, noteChanged, const QString&)

    SIGNAL_IMPL(AboutFriend, autoAcceptDirChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, autoAcceptCallChanged, IFriendSettings::AutoAcceptCallFlags)
    SIGNAL_IMPL(AboutFriend, autoGroupInviteChanged, bool)

private:
    const Friend* const f;
    IFriendSettings* const settings;
};
