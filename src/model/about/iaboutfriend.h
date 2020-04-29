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

#include "util/interface.h"
#include "src/persistence/ifriendsettings.h"

#include <QObject>

class IAboutFriend
{
public:
    virtual ~IAboutFriend() = default;
    virtual QString getName() const = 0;
    virtual QString getStatusMessage() const = 0;
    virtual ToxPk getPublicKey() const = 0;

    virtual QPixmap getAvatar() const = 0;

    virtual QString getNote() const = 0;
    virtual void setNote(const QString& note) = 0;

    virtual QString getAutoAcceptDir() const = 0;
    virtual void setAutoAcceptDir(const QString& path) = 0;

    virtual IFriendSettings::AutoAcceptCallFlags getAutoAcceptCall() const = 0;
    virtual void setAutoAcceptCall(IFriendSettings::AutoAcceptCallFlags flag) = 0;

    virtual bool getAutoGroupInvite() const = 0;
    virtual void setAutoGroupInvite(bool enabled) = 0;

    virtual bool clearHistory() = 0;
    virtual bool isHistoryExistence() = 0;

    /* signals */
    DECLARE_SIGNAL(nameChanged, const QString&);
    DECLARE_SIGNAL(statusChanged, const QString&);
    DECLARE_SIGNAL(publicKeyChanged, const QString&);

    DECLARE_SIGNAL(avatarChanged, const QPixmap&);
    DECLARE_SIGNAL(noteChanged, const QString&);

    DECLARE_SIGNAL(autoAcceptDirChanged, const QString&);
    DECLARE_SIGNAL(autoAcceptCallChanged, IFriendSettings::AutoAcceptCallFlags);
    DECLARE_SIGNAL(autoGroupInviteChanged, bool);
};
