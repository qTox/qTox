#include "userdb.h"

#include <QDebug>

#include "src/persistence/settings.h"
#include "src/persistence/db/rawdatabase.h"

/**
 * @brief Opens the profile database and prepares to work with the history.
 * @param profileName Profile name to load.
 * @param password If empty, the database will be opened unencrypted.
 */
UserDb::UserDb(const QString &profileName, const QString &password)
    : RawDatabase(getDbPath(profileName), password)
{
    init();
}

UserDb::~UserDb()
{
    // We could have execLater requests pending with a lambda attached,
    // so clear the pending transactions first
    sync();
}

/**
 * @brief Moves the database file on disk to match the new name.
 * @param newName New name.
 */
void UserDb::rename(const QString &newName)
{
    RawDatabase::rename(getDbPath(newName));
}

/**
 * @brief Retrieves the path to the database file for a given profile.
 * @param profileName Profile name.
 * @return Path to database.
 */
QString UserDb::getDbPath(const QString &profileName)
{
    return Settings::getInstance().getSettingsDirPath() + profileName + ".db";
}

/**
 * @brief Makes sure the history tables are created
 */
void UserDb::init()
{
    if (!isOpen())
    {
        qWarning() << "Database not open, init failed";
        return;
    }

    execLater("CREATE TABLE IF NOT EXISTS peers "
              "       (id INTEGER PRIMARY KEY, public_key TEXT NOT NULL UNIQUE); "
              "CREATE TABLE IF NOT EXISTS aliases "
              "       (id INTEGER PRIMARY KEY, owner INTEGER, "
              "        display_name BLOB NOT NULL, UNIQUE(owner, display_name));"
              "CREATE TABLE IF NOT EXISTS history "
              "       (id INTEGER PRIMARY KEY, timestamp INTEGER NOT NULL, "
              "        chat_id INTEGER NOT NULL, sender_alias INTEGER NOT NULL,"
              "        message BLOB NOT NULL);"
              "CREATE TABLE IF NOT EXISTS faux_offline_pending "
              "       (id INTEGER PRIMARY KEY);"
              );

}
