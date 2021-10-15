/*
    Copyright © 2017-2019 by The qTox Project Contributors

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

class ToxId;

class IProfileInfo
{
public:
    enum class RenameResult {
        OK, EmptyName, ProfileAlreadyExists, Error
    };

    enum class SaveResult {
        OK, EmptyPath, NoWritePermission, Error
    };

    enum class SetAvatarResult {
        OK, EmptyPath, CanNotOpen, CanNotRead, TooLarge
    };

    IProfileInfo() = default;
    virtual ~IProfileInfo();
    IProfileInfo(const IProfileInfo&) = default;
    IProfileInfo& operator=(const IProfileInfo&) = default;
    IProfileInfo(IProfileInfo&&) = default;
    IProfileInfo& operator=(IProfileInfo&&) = default;

    virtual bool setPassword(const QString& password) = 0;
    virtual bool deletePassword() = 0;
    virtual bool isEncrypted() const = 0;

    virtual void copyId() const = 0;

    virtual void setUsername(const QString& name) = 0;
    virtual void setStatusMessage(const QString& status) = 0;

    virtual QString getProfileName() const = 0;
    virtual RenameResult renameProfile(const QString& name) = 0;
    virtual SaveResult exportProfile(const QString& path) const = 0;
    virtual QStringList removeProfile() = 0;
    virtual void logout() = 0;

    virtual void copyQr(const QImage& image) const = 0;
    virtual SaveResult saveQr(const QImage& image, const QString& path) const = 0;

    virtual SetAvatarResult setAvatar(const QString& path) = 0;
    virtual void removeAvatar() = 0;

    DECLARE_SIGNAL(idChanged, const ToxId&);
    DECLARE_SIGNAL(usernameChanged, const QString&);
    DECLARE_SIGNAL(statusMessageChanged, const QString&);
};
