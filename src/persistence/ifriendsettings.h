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

#include <QObject>
#include <QFlag>

class ToxPk;

class IFriendSettings
{
public:
    enum class AutoAcceptCall
    {
        None = 0x00,
        Audio = 0x01,
        Video = 0x02,
        AV = Audio | Video
    };
    Q_DECLARE_FLAGS(AutoAcceptCallFlags, AutoAcceptCall)

    IFriendSettings() = default;
    virtual ~IFriendSettings();
    IFriendSettings(const IFriendSettings&) = default;
    IFriendSettings& operator=(const IFriendSettings&) = default;
    IFriendSettings(IFriendSettings&&) = default;
    IFriendSettings& operator=(IFriendSettings&&) = default;

    virtual QString getContactNote(const ToxPk& pk) const = 0;
    virtual void setContactNote(const ToxPk& pk, const QString& note) = 0;

    virtual QString getAutoAcceptDir(const ToxPk& pk) const = 0;
    virtual void setAutoAcceptDir(const ToxPk& pk, const QString& dir) = 0;

    virtual AutoAcceptCallFlags getAutoAcceptCall(const ToxPk& pk) const = 0;
    virtual void setAutoAcceptCall(const ToxPk& pk, AutoAcceptCallFlags accept) = 0;

    virtual bool getAutoGroupInvite(const ToxPk& pk) const = 0;
    virtual void setAutoGroupInvite(const ToxPk& pk, bool accept) = 0;

    virtual QString getFriendAlias(const ToxPk& pk) const = 0;
    virtual void setFriendAlias(const ToxPk& pk, const QString& alias) = 0;

    virtual int getFriendCircleID(const ToxPk& pk) const = 0;
    virtual void setFriendCircleID(const ToxPk& pk, int circleID) = 0;

    virtual QDateTime getFriendActivity(const ToxPk& pk) const = 0;
    virtual void setFriendActivity(const ToxPk& pk, const QDateTime& date) = 0;

    virtual void saveFriendSettings(const ToxPk& pk) = 0;
    virtual void removeFriendSettings(const ToxPk& pk) = 0;

signals:
    DECLARE_SIGNAL(autoAcceptCallChanged, const ToxPk& pk, AutoAcceptCallFlags accept);
    DECLARE_SIGNAL(autoGroupInviteChanged, const ToxPk& pk, bool accept);
    DECLARE_SIGNAL(autoAcceptDirChanged, const ToxPk& pk, const QString& dir);
    DECLARE_SIGNAL(contactNoteChanged, const ToxPk& pk, const QString& note);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(IFriendSettings::AutoAcceptCallFlags)
