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

#include <QObject>
#include "util/interface.h"
#include "src/core/toxpk.h"
#include "iprofileinfo.h"

class Core;
class QFile;
class QPoint;
class Profile;

class ProfileInfo : public QObject, public IProfileInfo
{
    Q_OBJECT
public:
    ProfileInfo(Core* core, Profile* profile);

    bool setPassword(const QString& password) override;
    bool deletePassword() override;
    bool isEncrypted() const override;

    void copyId() const override;

    void setUsername(const QString& name) override;
    void setStatusMessage(const QString& status) override;

    QString getProfileName() const override;
    RenameResult renameProfile(const QString& name) override;
    SaveResult exportProfile(const QString& path) const override;
    QStringList removeProfile() override;
    void logout() override;

    void copyQr(const QImage& image) const override;
    SaveResult saveQr(const QImage& image, const QString& path) const override;

    SetAvatarResult setAvatar(const QString& path) override;
    void removeAvatar() override;

    SIGNAL_IMPL(ProfileInfo, idChanged, const ToxId& id)
    SIGNAL_IMPL(ProfileInfo, usernameChanged, const QString& name)
    SIGNAL_IMPL(ProfileInfo, statusMessageChanged, const QString& message)

private:
    IProfileInfo::SetAvatarResult createAvatarFromFile(QFile& file, QByteArray& avatar);
    IProfileInfo::SetAvatarResult byteArrayToPng(QByteArray inData, QByteArray& outPng);
    IProfileInfo::SetAvatarResult scalePngToAvatar(QByteArray& avatar);
    Profile* const profile;
    Core* const core;
};
