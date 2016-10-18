#ifndef USER_INFO_DB_H
#define USER_INFO_DB_H

#include <QString>

#include "rawdatabase.h"

class UserDb : public RawDatabase
{   
public:
    UserDb(const QString& profileName, const QString& password);
    ~UserDb();

    void rename(const QString& newName);

    static QString getDbPath(const QString& profileName);

protected:
    void init();
};

#endif // USER_INFO_DB_H
