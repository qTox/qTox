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


#ifndef TOXID_H
#define TOXID_H

#include <QString>

/*!
 * \brief This class represents a Tox ID as specified at:
 *        https://libtoxcore.so/core_concepts.html
 */
class ToxId
{
public:
    ToxId(); ///< The default constructor. Creates an empty Tox ID.
    ToxId(const ToxId& other); ///< The copy constructor.
    ToxId(const QString& id); ///< Create a Tox ID from QString.
                              /// If the given id is not a valid Tox ID, then:
                              /// publicKey == id and noSpam == "" == checkSum.

    bool operator==(const ToxId& other) const; ///< Compares only publicKey.
    bool operator!=(const ToxId& other) const; ///< Compares only publicKey.
    bool isActiveProfile() const; ///< Returns true if this Tox ID is equals to
                                  /// the Tox ID of the currently active profile.
    QString toString() const; ///< Returns the Tox ID as QString.
    void clear(); ///< Clears all elements of the Tox ID.

    static bool isToxId(const QString& id); ///< Returns true if id is a valid Tox ID.

public:
    QString publicKey;
    QString noSpam;
    QString checkSum;
};

#endif // TOXID_H
