/*
    Copyright © 2015-2018 by The qTox Project Contributors

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

#include "src/core/core.h"
#include "src/core/toxencrypt.h"
#include "src/core/toxid.h"

#include "src/persistence/history.h"

#include <QByteArray>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QVector>
#include <memory>

class Profile : public QObject
{
    Q_OBJECT

public:
    static Profile* loadProfile(QString name, const QString& password = QString());
    static Profile* createProfile(QString name, QString password);
    ~Profile();

    Core* getCore();
    QString getName() const;

    void startCore();
    void restartCore();
    bool isNewProfile();
    bool isEncrypted() const;
    QString setPassword(const QString& newPassword);
    const ToxEncrypt* getPasskey() const;

    QPixmap loadAvatar();
    QPixmap loadAvatar(const ToxPk& owner);
    QByteArray loadAvatarData(const ToxPk& owner);
    void setAvatar(QByteArray pic);
    QByteArray getAvatarHash(const ToxPk& owner);
    void removeSelfAvatar();

    bool isHistoryEnabled();
    History* getHistory();

    QStringList remove();

    bool rename(QString newName);

    static void scanProfiles();
    static QStringList getProfiles();

    static bool exists(QString name);
    static bool isEncrypted(QString name);
    static QString getDbPath(const QString& profileName);

signals:
    void selfAvatarChanged(const QPixmap& pixmap);

    // TODO(sudden6): this doesn't seem to be the right place for Core errors
    void failedToStart();
    void badProxy();

public slots:
    void onRequestSent(const ToxPk& friendPk, const QString& message);

private slots:
    void loadDatabase(const ToxId& id, QString password);
    void saveAvatar(const ToxPk& owner, const QByteArray& avatar);
    void removeAvatar(const ToxPk& owner);
    void onSaveToxSave();
    // TODO(sudden6): use ToxPk instead of friendId
    void onAvatarOfferReceived(uint32_t friendId, uint32_t fileId, const QByteArray& avatarHash);

private:
    Profile(QString name, const QString& password, bool newProfile, const QByteArray& toxsave);
    static QStringList getFilesByExt(QString extension);
    QString avatarPath(const ToxPk& owner, bool forceUnencrypted = false);
    bool saveToxSave(QByteArray data);
    void initCore(const QByteArray& toxsave, ICoreSettings& s);

private:
    std::unique_ptr<Core> core = nullptr;
    QString name;
    std::unique_ptr<ToxEncrypt> passkey = nullptr;
    std::shared_ptr<RawDatabase> database;
    std::unique_ptr<History> history;
    bool newProfile;
    bool isRemoved;
    bool encrypted = false;
    static QStringList profiles;
};

#endif // PROFILE_H
