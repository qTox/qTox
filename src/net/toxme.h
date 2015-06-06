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


#ifndef TOXME_H
#define TOXME_H

#include <QString>
#include <QMutex>
#include <memory>
#include "src/core/toxid.h"

class QNetworkAccessManager;

/// This class implements a client for the toxme.se API
/// The class is thread safe
/// May process events while waiting for blocking calls
class Toxme
{
public:
    /// Converts a toxme.se address to a Tox ID, returns an empty ID on error
    static ToxId lookup(QString address);
    /// Creates a new toxme.se address associated with a Tox ID.
    /// If keepPrivate, the address will not be published on toxme.se
    /// The bio is a short optional description of yourself if you want to publish your address.
    static bool createAddress(ToxId id, QString address,
                              bool keepPrivate=true, QString bio=QString());
    /// Deletes the address associated with your current Tox ID
    static bool deleteAddress(ToxId id);

private:
    Toxme()=delete;
    static QByteArray makeJsonRequest(QString json);
    static QByteArray prepareEncryptedJson(int action, QString payload);
    static int extractError(QString json);

private:
    static const QString apiUrl;
    static const unsigned char pinnedPk[];
};

#endif // TOXME_H
