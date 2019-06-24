/*
    Copyright © 2017-2019 by The qTox Project Contributors

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

#ifndef TOXME_DATA_H
#define TOXME_DATA_H

#include <QByteArray>

#include "src/core/toxid.h"

class QString;

class ToxmeData
{
public:
    enum ExecCode
    {
        ExecError = -50,
        Ok = 0,
        Updated = 1,
        ServerError = 2,
        IncorrectResponse = 3,
        NoPassword = 4
    };

    QByteArray parsePublicKey(const QString& text) const;
    QString encryptedJson(int action, const QByteArray& pk, const QByteArray& encrypted,
                          const QByteArray& nonce) const;
    QString lookupRequest(const QString& address) const;
    ToxId lookup(const QString& text) const;

    ExecCode extractCode(const QString& json) const;
    QString createAddressRequest(const ToxId id, const QString& address, const QString& bio,
                                 bool keepPrivate) const;
    QString getPass(const QString& json, ToxmeData::ExecCode& code);
    QString deleteAddressRequest(const ToxPk& pk);
};

#endif // TOXME_DATA_H
