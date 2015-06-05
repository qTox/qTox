/*
    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef PLAINDB_H
#define PLAINDB_H

#include "genericddinterface.h"

#include <QSqlDatabase>

namespace Db {
    enum class syncType : int {stOff = 0, stNormal = 1, stFull = 2};
}

class PlainDb : public GenericDdInterface
{
public:
    PlainDb(const QString &db_name, QList<QString> initList);
    virtual ~PlainDb();

    virtual QSqlQuery exec(const QString &query);

private:
    QSqlDatabase *db;
};

#endif // PLAINDB_H
