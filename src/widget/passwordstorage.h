/*
    Copyright © 2016 by The qTox Project

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


#ifndef PASSWORDSTORAGE_H
#define PASSWORDSTORAGE_H

#include <qt5keychain/keychain.h>

class ReadToXPasswordJob : public QKeychain::ReadPasswordJob {
public:
    explicit ReadToXPasswordJob(const QString& profileName, QObject *parent = nullptr);

    const QString password() const;
};

class WriteToXPasswordJob : public QKeychain::WritePasswordJob {
public:
    explicit WriteToXPasswordJob(const QString& profileName, const QString& password, QObject *parent = nullptr);
};

class DeleteToXPasswordJob : public QKeychain::DeletePasswordJob {
public:
    explicit DeleteToXPasswordJob(const QString& profileName, QObject *parent = nullptr);
};

#endif // PASSWORDSTORAGE_H
