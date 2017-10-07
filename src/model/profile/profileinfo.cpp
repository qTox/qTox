/*
    Copyright © 2017 by The qTox Project Contributors

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

#include "profileinfo.h"
#include "src/core/core.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/nexus.h"

#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QBuffer>

ProfileInfo::ProfileInfo(Core* core, Profile *profile)
    : profile{profile}
    , core{core}
{
    connect(core, &Core::idSet, this, &ProfileInfo::idChanged);
    connect(core, &Core::usernameSet, this, &ProfileInfo::usernameChanged);
    connect(core, &Core::statusMessageSet, this, &ProfileInfo::usernameChanged);
}

bool ProfileInfo::setPassword(const QString &password)
{
    QString errorMsg = profile->setPassword(password);
    return errorMsg.isEmpty();
}

bool ProfileInfo::deletePassword()
{
    QString errorMsg = profile->setPassword("");
    return errorMsg.isEmpty();
}

bool ProfileInfo::isEncrypted() const
{
    return profile->isEncrypted();
}

void ProfileInfo::copyId() const
{
    ToxId selfId = profile->getCore()->getSelfId();
    QString txt = selfId.toString();
    QClipboard* clip = QApplication::clipboard();
    clip->setText(txt, QClipboard::Clipboard);
    if (clip->supportsSelection()) {
        clip->setText(txt, QClipboard::Selection);
    }
}

void ProfileInfo::setUsername(const QString &name)
{
    profile->getCore()->setUsername(name);
}

void ProfileInfo::setStatusMessage(const QString &status)
{
    profile->getCore()->setStatusMessage(status);
}

QString ProfileInfo::getProfileName() const
{
    return profile->getName();
}

static QString sanitize(const QString& src)
{
    QString name = src;
    // these are pretty much Windows banned filename characters
    QList<QChar> banned{'/', '\\', ':', '<', '>', '"', '|', '?', '*'};
    for (QChar c : banned) {
        name.replace(c, '_');
    }

    // also remove leading and trailing periods
    if (name[0] == '.') {
        name[0] = '_';
    }

    if (name.endsWith('.')) {
        name[name.length() - 1] = '_';
    }

    return name;
}

IProfileInfo::RenameResult ProfileInfo::renameProfile(const QString &name)
{
    QString cur = profile->getName();
    if (name.isEmpty()) {
        return RenameResult::OK;
    }

    QString newName = sanitize(name);

    if (Profile::exists(newName)) {
        return RenameResult::ProfileAlreadyExists;
    }

    if (!profile->rename(name)) {
        return RenameResult::Error;
    }

    return RenameResult::OK;
}

/**
 * @brief Dangerous way to find out if a path is writable.
 * @param filepath Path to file which should be deleted.
 * @return True, if file writeable, false otherwise.
 */
static bool tryRemoveFile(const QString& filepath)
{
    QFile tmp(filepath);
    bool writable = tmp.open(QIODevice::WriteOnly);
    tmp.remove();
    return writable;
}

IProfileInfo::SaveResult ProfileInfo::exportProfile(const QString &path) const
{
    QString current = profile->getName() + Core::TOX_EXT;
    if (path.isEmpty()) {
        return SaveResult::OK;
    }

    if (!tryRemoveFile(path)) {
        return SaveResult::NoWritePermission;
    }

    if (!QFile::copy(Settings::getInstance().getSettingsDirPath() + current, path)) {
        return SaveResult::Error;
    }

    return SaveResult::OK;
}

QVector<QString> ProfileInfo::removeProfile()
{
    QVector<QString> manualDeleteFiles = profile->remove();
    Nexus::getInstance().showLoginLater();
    return manualDeleteFiles;
}

void ProfileInfo::layout()
{
    Settings::getInstance().saveGlobal();
    Nexus::getInstance().showLoginLater();
}

void ProfileInfo::copyQr(const QImage& image) const
{
    QApplication::clipboard()->setImage(image);
}

IProfileInfo::SaveResult ProfileInfo::saveQr(const QImage& image, const QString& path) const
{
    QString current = profile->getName() + ".png";
    if (path.isEmpty()) {
        return SaveResult::OK;
    }

    if (!tryRemoveFile(path)) {
        return SaveResult::NoWritePermission;
    }

    // nullptr - image format same as file extension,
    // 75-quality, png file is ~6.3kb
    if (!image.save(path, nullptr, 75)) {
        return SaveResult::Error;
    }

    return SaveResult::OK;
}

QByteArray picToPng(const QPixmap& pic)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    pic.save(&buffer, "PNG");
    buffer.close();
    return bytes;
};

IProfileInfo::SetAvatarResult ProfileInfo::setAvatar(const QString &path)
{
    if (path.isEmpty()) {
        return SetAvatarResult::OK;
    }

    QFile file(path);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        return SetAvatarResult::CanNotOpen;
    }

    QPixmap pic;
    if (!pic.loadFromData(file.readAll())) {
        return SetAvatarResult::CanNotRead;
    }

    // Limit the avatar size to 64kB
    // We do a first rescale to 256x256 in case the image was huge, then keep tryng from here
    // TODO: Refactor
    QByteArray bytes{picToPng(pic)};
    if (bytes.size() > 65535) {
        pic = pic.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        bytes = picToPng(pic);
    }

    if (bytes.size() > 65535) {
        bytes = picToPng(pic.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    if (bytes.size() > 65535) {
        bytes = picToPng(pic.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    if (bytes.size() > 65535) {
        bytes = picToPng(pic.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // If this happens, you're really doing it on purpose.
    if (bytes.size() > 65535) {
        return SetAvatarResult::TooLarge;
    }

    profile->setAvatar(bytes, core->getSelfPublicKey().toString());
    return SetAvatarResult::OK;
}

void ProfileInfo::removeAvatar()
{
    profile->removeAvatar();
}
