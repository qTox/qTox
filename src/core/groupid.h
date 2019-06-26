/*
    Copyright © 2019 by The qTox Project Contributors

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

#ifndef GROUPID_H
#define GROUPID_H

#include "src/core/contactid.h"
#include <QByteArray>
#include <cstdint>

class GroupId : public ContactId
{
public:
    GroupId();
    GroupId(const GroupId& other);
    explicit GroupId(const QByteArray& rawId);
    explicit GroupId(const uint8_t* rawId);
    int getSize() const override;
};

#endif // GROUPID_H
