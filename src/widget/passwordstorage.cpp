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

#ifdef QTOX_QTKEYCHAIN

#include "passwordstorage.h"

static const QString serviceName(QStringLiteral("qToX"));

ReadToXPasswordJob::ReadToXPasswordJob(const QString &profileName, QObject *parent)
    : QKeychain::ReadPasswordJob(serviceName, parent){
    setAutoDelete(true);
    setKey(profileName);
}

const QString ReadToXPasswordJob::password() const{
    return textData();
}

WriteToXPasswordJob::WriteToXPasswordJob(const QString &profileName, const QString &password, QObject *parent)
    : QKeychain::WritePasswordJob(serviceName, parent){
    setAutoDelete(true);
    setKey(profileName);
    setTextData(password);
}

DeleteToXPasswordJob::DeleteToXPasswordJob(const QString &profileName, QObject *parent)
    : QKeychain::DeletePasswordJob(serviceName, parent){
    setAutoDelete(true);
    setKey(profileName);
}

#endif // QTOX_QTKEYCHAIN
