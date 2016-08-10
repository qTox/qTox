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


#include "profile.h"
#include "profilelocker.h"
#include "src/persistence/settings.h"
#include "src/persistence/historykeeper.h"
#include "src/core/core.h"
#include "src/widget/gui.h"
#include "src/widget/widget.h"
#include "src/nexus.h"
#include <cassert>
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <QThread>
#include <QObject>
#include <QDebug>
#include <sodium.h>

/**
@class Profile
@brief Manages user profiles.

@var bool Profile::newProfile
@brief True if this is a newly created profile, with no .tox save file yet.

@var bool Profile::isRemoved
@brief True if the profile has been removed by remove().

@var static constexpr int Profile::encryptHeaderSize = 8
@brief How much data we need to read to check if the file is encrypted.
@note Must be >= TOX_ENC_SAVE_MAGIC_LENGTH (8), which isn't publicly defined.
*/

QVector<QString> Profile::profiles;

Profile::Profile(QString name, const QString &password, bool isNewProfile)
    : name{name}, password{password},
      newProfile{isNewProfile}, isRemoved{false}
{
    if (!password.isEmpty())
        passkey = *core->createPasskey(password);

    Settings& s = Settings::getInstance();
    s.setCurrentProfile(name);
    s.saveGlobal();

    // At this point it's too early to load the personal settings (Nexus will do it), so we always load
    // the history, and if it fails we can't change the setting now, but we keep a nullptr
    history.reset(new History{name, password});
    if (!history->isValid())
    {
        qWarning() << "Failed to open history for profile"<<name;
        GUI::showError(QObject::tr("Error"), QObject::tr("qTox couldn't open your chat logs, they will be disabled."));
        history.release();
    }

    coreThread = new QThread();
    coreThread->setObjectName("qTox Core");
    core = new Core(coreThread, *this);
    core->moveToThread(coreThread);
    QObject::connect(coreThread, &QThread::started, core, &Core::start);
}

/**
@brief Locks and loads an existing profile and creates the associate Core* instance.
@param name Profile name.
@param password Profile password.
@return Returns a nullptr on error. Profile pointer otherwise.

@example If the profile is already in use return nullptr.
*/
Profile* Profile::loadProfile(QString name, const QString &password)
{
    if (ProfileLocker::hasLock())
    {
        qCritical() << "Tried to load profile "<<name<<", but another profile is already locked!";
        return nullptr;
    }

    if (!ProfileLocker::lock(name))
    {
        qWarning() << "Failed to lock profile "<<name;
        return nullptr;
    }

    // Check password
    {
        QString path = Settings::getInstance().getSettingsDirPath() + name + ".tox";
        QFile saveFile(path);
        qDebug() << "Loading tox save "<<path;

        if (!saveFile.exists())
        {
            qWarning() << "The tox save file "<<path<<" was not found";
            ProfileLocker::unlock();
            return nullptr;
        }

        if (!saveFile.open(QIODevice::ReadOnly))
        {
            qCritical() << "The tox save file " << path << " couldn't' be opened";
            ProfileLocker::unlock();
            return nullptr;
        }

        qint64 fileSize = saveFile.size();
        if (fileSize <= 0)
        {
            qWarning() << "The tox save file"<<path<<" is empty!";
            ProfileLocker::unlock();
            return nullptr;
        }

        QByteArray data = saveFile.readAll();
        if (tox_is_data_encrypted((uint8_t*)data.data()))
        {
            if (password.isEmpty())
            {
                qCritical() << "The tox save file is encrypted, but we don't have a password!";
                ProfileLocker::unlock();
                return nullptr;
            }

            uint8_t salt[TOX_PASS_SALT_LENGTH];
            tox_get_salt(reinterpret_cast<uint8_t *>(data.data()), salt);
            auto tmpkey = *Core::createPasskey(password, salt);

            data = Core::decryptData(data, tmpkey);
            if (data.isEmpty())
            {
                qCritical() << "Failed to decrypt the tox save file";
                ProfileLocker::unlock();
                return nullptr;
            }
        }
        else
        {
            if (!password.isEmpty())
                qWarning() << "We have a password, but the tox save file is not encrypted";
        }
    }

    Profile* p = new Profile(name, password, false);
    if (p->history && HistoryKeeper::isFileExist(!password.isEmpty()))
        p->history->import(*HistoryKeeper::getInstance(*p));
    return p;
}

/**
@brief Creates a new profile and the associated Core* instance.
@param name Username.
@param password If password is not empty, the profile will be encrypted.
@return Returns a nullptr on error. Profile pointer otherwise.

@example If the profile is already in use return nullptr.
*/
Profile* Profile::createProfile(QString name, QString password)
{
    if (ProfileLocker::hasLock())
    {
        qCritical() << "Tried to create profile "<<name<<", but another profile is already locked!";
        return nullptr;
    }

    if (exists(name))
    {
        qCritical() << "Tried to create profile "<<name<<", but it already exists!";
        return nullptr;
    }

    if (!ProfileLocker::lock(name))
    {
        qWarning() << "Failed to lock profile "<<name;
        return nullptr;
    }

    Settings::getInstance().createPersonal(name);
    return new Profile(name, password, true);
}

Profile::~Profile()
{
    if (!isRemoved && core->isReady())
        saveToxSave();
    delete core;
    delete coreThread;
    if (!isRemoved)
    {
        Settings::getInstance().savePersonal(this);
        Settings::getInstance().sync();
        ProfileLocker::assertLock();
        assert(ProfileLocker::getCurLockName() == name);
        ProfileLocker::unlock();
    }
}

/**
@brief Lists all the files in the config dir with a given extension
@param extension Raw extension, e.g. "jpeg" not ".jpeg".
@return Vector of filenames.
*/
QVector<QString> Profile::getFilesByExt(QString extension)
{
    QDir dir(Settings::getInstance().getSettingsDirPath());
    QVector<QString> out;
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    dir.setNameFilters(QStringList("*."+extension));
    QFileInfoList list = dir.entryInfoList();
    out.reserve(list.size());
    for (QFileInfo file : list)
        out += file.completeBaseName();
    return out;
}

/**
@brief Scan for profile, automatically importing them if needed.
@warning NOT thread-safe.
*/
void Profile::scanProfiles()
{
    profiles.clear();
    QVector<QString> toxfiles = getFilesByExt("tox"), inifiles = getFilesByExt("ini");
    for (QString toxfile : toxfiles)
    {
        if (!inifiles.contains(toxfile))
            Settings::getInstance().createPersonal(toxfile);
        profiles.append(toxfile);
    }
}

QVector<QString> Profile::getProfiles()
{
    return profiles;
}

Core* Profile::getCore()
{
    return core;
}

QString Profile::getName() const
{
    return name;
}

/**
@brief Starts the Core thread
*/
void Profile::startCore()
{
    coreThread->start();
}

bool Profile::isNewProfile()
{
    return newProfile;
}

/**
@brief Loads the profile's .tox save from file, unencrypted
@return Byte array of loaded profile save.
*/
QByteArray Profile::loadToxSave()
{
    assert(!isRemoved);
    QByteArray data;

    QString path = Settings::getInstance().getSettingsDirPath() + name + ".tox";
    QFile saveFile(path);
    qint64 fileSize;
    qDebug() << "Loading tox save "<<path;

    if (!saveFile.exists())
    {
        qWarning() << "The tox save file "<<path<<" was not found";
        goto fail;
    }

    if (!saveFile.open(QIODevice::ReadOnly))
    {
        qCritical() << "The tox save file " << path << " couldn't' be opened";
        goto fail;
    }

    fileSize = saveFile.size();
    if (fileSize <= 0)
    {
        qWarning() << "The tox save file"<<path<<" is empty!";
        goto fail;
    }

    data = saveFile.readAll();
    if (tox_is_data_encrypted((uint8_t*)data.data()))
    {
        if (password.isEmpty())
        {
            qCritical() << "The tox save file is encrypted, but we don't have a password!";
            data.clear();
            goto fail;
        }

        uint8_t salt[TOX_PASS_SALT_LENGTH];
        tox_get_salt(reinterpret_cast<uint8_t *>(data.data()), salt);
        passkey = *core->createPasskey(password, salt);

        data = core->decryptData(data, passkey);
        if (data.isEmpty())
            qCritical() << "Failed to decrypt the tox save file";
    }
    else
    {
        if (!password.isEmpty())
            qWarning() << "We have a password, but the tox save file is not encrypted";
    }

fail:
    saveFile.close();
    return data;
}

/**
@brief Saves the profile's .tox save, encrypted if needed.
@warning Invalid on deleted profiles.
*/
void Profile::saveToxSave()
{
    assert(core->isReady());
    QByteArray data = core->getToxSaveData();
    assert(data.size());
    saveToxSave(data);
}

/**
@brief Write the .tox save, encrypted if needed.
@param data Byte array of profile save.
@warning Invalid on deleted profiles.
*/
void Profile::saveToxSave(QByteArray data)
{
    assert(!isRemoved);
    ProfileLocker::assertLock();
    assert(ProfileLocker::getCurLockName() == name);

    QString path = Settings::getInstance().getSettingsDirPath() + name + ".tox";
    qDebug() << "Saving tox save to "<<path;
    QSaveFile saveFile(path);
    if (!saveFile.open(QIODevice::WriteOnly))
    {
        qCritical() << "Tox save file " << path << " couldn't be opened";
        return;
    }

    if (!password.isEmpty())
    {
        passkey = *core->createPasskey(password);
        data = core->encryptData(data, passkey);
        if (data.isEmpty())
        {
            qCritical() << "Failed to encrypt, can't save!";
            saveFile.cancelWriting();
            return;
        }
    }

    saveFile.write(data);

    // check if everything got written
    if (saveFile.flush())
    {
        saveFile.commit();
        newProfile = false;
    }
    else
    {
        saveFile.cancelWriting();
        qCritical() << "Failed to write, can't save!";
    }
}

/**
@brief Gets the path of the avatar file cached by this profile and corresponding to this owner ID.
@param ownerId Path to avatar of friend with this ID will returned.
@param forceUnencrypted If true, return the path to the plaintext file even if this is an encrypted profile.
@return Path to the avatar.
*/
QString Profile::avatarPath(const QString &ownerId, bool forceUnencrypted)
{
    if (password.isEmpty() || forceUnencrypted)
        return Settings::getInstance().getSettingsDirPath() + "avatars/" + ownerId + ".png";

    QByteArray idData = ownerId.toUtf8();
    QByteArray pubkeyData = core->getSelfId().publicKey.toUtf8();
    constexpr int hashSize = TOX_PUBLIC_KEY_SIZE;
    static_assert(hashSize >= crypto_generichash_BYTES_MIN
                  && hashSize <= crypto_generichash_BYTES_MAX, "Hash size not supported by libsodium");
    static_assert(hashSize >= crypto_generichash_KEYBYTES_MIN
                  && hashSize <= crypto_generichash_KEYBYTES_MAX, "Key size not supported by libsodium");
    QByteArray hash(hashSize, 0);
    crypto_generichash((uint8_t*)hash.data(), hashSize, (uint8_t*)idData.data(), idData.size(), (uint8_t*)pubkeyData.data(), pubkeyData.size());
    return Settings::getInstance().getSettingsDirPath() + "avatars/" + hash.toHex().toUpper() + ".png";
}

/**
@brief Get our avatar from cache.
@return Avatar as QPixmap.
*/
QPixmap Profile::loadAvatar()
{
    return loadAvatar(core->getSelfId().publicKey);
}

/**
@brief Get a contact's avatar from cache.
@param ownerId Friend ID to load avatar.
@return Avatar as QPixmap.
*/
QPixmap Profile::loadAvatar(const QString &ownerId)
{
    QPixmap pic;
    pic.loadFromData(loadAvatarData(ownerId));
    return pic;
}

/**
@brief Get a contact's avatar from cache
@param ownerId Friend ID to load avatar.
@return Avatar as QByteArray.
*/
QByteArray Profile::loadAvatarData(const QString &ownerId)
{
  return loadAvatarData(ownerId, password);
}

/**
@brief Get a contact's avatar from cache, with a specified profile password.
@param ownerId Friend ID to load avatar.
@param password Profile password to decrypt data.
@return Avatar as QByteArray.
*/
QByteArray Profile::loadAvatarData(const QString &ownerId, const QString &password)
{
    QString path = avatarPath(ownerId);
    bool encrypted = !password.isEmpty();

    // If the encrypted avatar isn't found, try loading the unencrypted one for the same ID
    if (!password.isEmpty() && !QFile::exists(path))
    {
        encrypted = false;
        path = avatarPath(ownerId, true);
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QByteArray pic = file.readAll();
    if (encrypted && !pic.isEmpty())
    {
        uint8_t salt[TOX_PASS_SALT_LENGTH];
        tox_get_salt(reinterpret_cast<uint8_t *>(pic.data()), salt);
        auto passkey = core->createPasskey(password, salt);
        pic = core->decryptData(pic, *passkey);
    }
    return pic;
}

/**
@brief Save an avatar to cache.
@param pic Picture to save.
@param ownerId ID of avatar owner.
*/
void Profile::saveAvatar(QByteArray pic, const QString &ownerId)
{
    if (!password.isEmpty() && !pic.isEmpty())
        pic = core->encryptData(pic, passkey);

    QString path = avatarPath(ownerId);
    QDir(Settings::getInstance().getSettingsDirPath()).mkdir("avatars");
    if (pic.isEmpty())
    {
        QFile::remove(path);
    }
    else
    {
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly))
        {
            qWarning() << "Tox avatar " << path << " couldn't be saved";
            return;
        }
        file.write(pic);
        file.commit();
    }
}

/**
@brief Get the tox hash of a cached avatar.
@param ownerId Friend ID to get hash.
@return Avatar tox hash.
*/
QByteArray Profile::getAvatarHash(const QString &ownerId)
{
    QByteArray pic = loadAvatarData(ownerId);
    QByteArray avatarHash(TOX_HASH_LENGTH, 0);
    tox_hash((uint8_t*)avatarHash.data(), (uint8_t*)pic.data(), pic.size());
    return avatarHash;
}

/**
@brief Removes our own avatar.
*/
void Profile::removeAvatar()
{
    removeAvatar(core->getSelfId().publicKey);
}

/**
@brief Checks that the history is enabled in the settings, and loaded successfully for this profile.
@return True if enabled, false otherwise.
*/
bool Profile::isHistoryEnabled()
{
    return Settings::getInstance().getEnableLogging() && history;
}

/**
@brief Get chat history.
@return May return a nullptr if the history failed to load.
*/
History *Profile::getHistory()
{
    return history.get();
}

/**
@brief Removes a cached avatar.
@param ownerId Friend ID whose avater to delete.
*/
void Profile::removeAvatar(const QString &ownerId)
{
    QFile::remove(avatarPath(ownerId));
    if (ownerId == core->getSelfId().publicKey)
        core->setAvatar({});
}

bool Profile::exists(QString name)
{
    QString path = Settings::getInstance().getSettingsDirPath() + name;
    return QFile::exists(path+".tox");
}

/**
@brief Checks, if profile has a password.
@return True if we have a password set (doesn't check the actual file on disk).
*/
bool Profile::isEncrypted() const
{
    return !password.isEmpty();
}

/**
@brief Checks if profile is encrypted.
@note Checks the actual file on disk.
@param name Profile name.
@return True if profile is encrypted, false otherwise.
*/
bool Profile::isEncrypted(QString name)
{
    uint8_t data[encryptHeaderSize] = {0};
    QString path = Settings::getInstance().getSettingsDirPath() + name + ".tox";
    QFile saveFile(path);
    if (!saveFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Couldn't open tox save "<<path;
        return false;
    }

    saveFile.read((char*)data, encryptHeaderSize);
    saveFile.close();

    return tox_is_data_encrypted(data);
}

/**
@brief Removes the profile permanently.
Updates the profiles vector.
@return Vector of filenames that could not be removed.
@warning It is invalid to call loadToxSave or saveToxSave on a deleted profile.
*/
QVector<QString> Profile::remove()
{
    if (isRemoved)
    {
        qWarning() << "Profile " << name << " is already removed!";
        return {};
    }
    isRemoved = true;

    qDebug() << "Removing profile" << name;
    for (int i=0; i<profiles.size(); i++)
    {
        if (profiles[i] == name)
        {
            profiles.removeAt(i);
            i--;
        }
    }
    QString path = Settings::getInstance().getSettingsDirPath() + name;
    ProfileLocker::unlock();

    QFile profileMain {path + ".tox"};
    QFile profileConfig {path + ".ini"};
    QFile historyLegacyUnencrypted {HistoryKeeper::getHistoryPath(name, 0)};
    QFile historyLegacyEncrypted {HistoryKeeper::getHistoryPath(name, 1)};

    QVector<QString> ret;

    if (!profileMain.remove() && profileMain.exists())
    {
        ret.push_back(profileMain.fileName());
        qWarning() << "Could not remove file " << profileMain.fileName();
    }
    if (!profileConfig.remove() && profileConfig.exists())
    {
        ret.push_back(profileConfig.fileName());
        qWarning() << "Could not remove file " << profileConfig.fileName();
    }

    if (!historyLegacyUnencrypted.remove() && historyLegacyUnencrypted.exists())
    {
        ret.push_back(historyLegacyUnencrypted.fileName());
        qWarning() << "Could not remove file " << historyLegacyUnencrypted.fileName();
    }
    if (!historyLegacyEncrypted.remove() && historyLegacyEncrypted.exists())
    {
        ret.push_back(historyLegacyEncrypted.fileName());
        qWarning() << "Could not remove file " << historyLegacyUnencrypted.fileName();
    }

    if (history)
    {
        if (!history->remove() && QFile::exists(History::getDbPath(name)))
        {
            ret.push_back(History::getDbPath(name));
            qWarning() << "Could not remove file " << History::getDbPath(name);
        }
        history.release();
    }

    return ret;
}

/**
@brief Tries to rename the profile.
@param newName New name for the profile.
@return False on error, true otherwise.
*/
bool Profile::rename(QString newName)
{
    QString path = Settings::getInstance().getSettingsDirPath() + name,
            newPath = Settings::getInstance().getSettingsDirPath() + newName;

    if (!ProfileLocker::lock(newName))
        return false;

    QFile::rename(path+".tox", newPath+".tox");
    QFile::rename(path+".ini", newPath+".ini");
    if (history)
        history->rename(newName);
    bool resetAutorun = Settings::getInstance().getAutorun();
    Settings::getInstance().setAutorun(false);
    Settings::getInstance().setCurrentProfile(newName);
    if (resetAutorun)
        Settings::getInstance().setAutorun(true); // fixes -p flag in autostart command line

    name = newName;
    return true;
}

/**
@brief Checks whether the password is valid.
@return True, if password is valid, false otherwise.
*/
bool Profile::checkPassword()
{
    if (isRemoved)
        return false;

    return !loadToxSave().isEmpty();
}

QString Profile::getPassword() const
{
    return password;
}

const TOX_PASS_KEY& Profile::getPasskey() const
{
    return passkey;
}

/**
@brief Delete core and restart a new one
*/
void Profile::restartCore()
{
    GUI::setEnabled(false); // Core::reset re-enables it
    if (!isRemoved && core->isReady())
        saveToxSave();
    QMetaObject::invokeMethod(core, "reset");
}

/**
@brief Changes the encryption password and re-saves everything with it
@param newPassword Password for encryption.
*/
void Profile::setPassword(const QString &newPassword)
{
    QByteArray avatar = loadAvatarData(core->getSelfId().publicKey);
    QString oldPassword = password;
    password = newPassword;
    passkey = *core->createPasskey(password);
    saveToxSave();

    if (history)
    {
        history->setPassword(newPassword);
        Nexus::getDesktopGUI()->reloadHistory();
    }
    saveAvatar(avatar, core->getSelfId().publicKey);

    QVector<uint32_t> friendList = core->getFriendList();
    QVectorIterator<uint32_t> i(friendList);
    while (i.hasNext())
    {
        QString friendPublicKey = core->getFriendPublicKey(i.next());
        saveAvatar(loadAvatarData(friendPublicKey,oldPassword),friendPublicKey);
    }
}
