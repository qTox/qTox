/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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

#include "plaindb.h"
#include <QDebug>
#include <QSqlQuery>
#include <QString>

PlainDb::PlainDb(const QString &db_name)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(db_name);

    if (!db.open())
    {
        qWarning() << QString("Can't open file: %1, history will not be saved!").arg(db_name);
        db.setDatabaseName(":memory:");
        db.open();
    }
}

PlainDb::~PlainDb()
{
    db.close();
}

QSqlQuery PlainDb::exec(const QString &query)
{
    return db.exec(query);
}

bool PlainDb::save()
{
    return true;
}
