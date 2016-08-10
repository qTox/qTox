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


#ifndef PROFILE_H
#define PROFILE_H

#include <QVector>
#include <QString>
#include <QByteArray>
#include <QPixmap>
#include <tox/toxencryptsave.h>
#include <memory>
#include "src/persistence/history.h"

class Core;
class QThread;

class Profile
{
public:
    static Profile* loadProfile(QString name, const QString &password = QString());
    static Profile* createProfile(QString name, QString password);
    ~Profile();

    Core* getCore();
    QString getName() const;

    void startCore();
    void restartCore();
    bool isNewProfile();
    bool isEncrypted() const;
    bool checkPassword();
    QString getPassword() const;
    void setPassword(const QString &newPassword);
    const TOX_PASS_KEY& getPasskey() const;

    QByteArray loadToxSave();
    void saveToxSave();
    void saveToxSave(QByteArray data);

    QPixmap loadAvatar();
    QPixmap loadAvatar(const QString& ownerId);
    QByteArray loadAvatarData(const QString& ownerId);
    QByteArray loadAvatarData(const QString& ownerId, const QString& password);
    void saveAvatar(QByteArray pic, const QString& ownerId);
    QByteArray getAvatarHash(const QString& ownerId);
    void removeAvatar(const QString& ownerId);
    void removeAvatar();

    bool isHistoryEnabled();
    History* getHistory();

    QVector<QString> remove();

    bool rename(QString newName);

    static void scanProfiles();
    static QVector<QString> getProfiles();

    static bool exists(QString name);
    static bool isEncrypted(QString name);

private:
    Profile(QString name, const QString &password, bool newProfile);
    static QVector<QString> getFilesByExt(QString extension);
    QString avatarPath(const QString& ownerId, bool forceUnencrypted = false);

private:
    Core* core;
    QThread* coreThread;
    QString name, password;
    TOX_PASS_KEY passkey;
    std::unique_ptr<History> history;
    bool newProfile;
    bool isRemoved;
    static QVector<QString> profiles;
    static constexpr int encryptHeaderSize = 8;
};

#endif // PROFILE_H
