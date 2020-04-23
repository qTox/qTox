/*
    Copyright © 2020 by The qTox Project Contributors

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

#ifndef TOXCLIENTSTANDARDS_H
#define TOXCLIENTSTANDARDS_H

#include <cstdint>

namespace ToxClientStandards
{
    // From TCS 2.2.4, max valid avatar size is 64KiB
    constexpr static uint64_t MaxAvatarSize = 64 * 1024;
    constexpr bool IsValidAvatarSize(uint64_t fileSize)
    {
        return fileSize <= MaxAvatarSize;
    }
} // ToxClientStandards

#endif // TOXCLIENTSTANDARDS_H
