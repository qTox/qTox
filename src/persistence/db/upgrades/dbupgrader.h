/*
    Copyright © 2022 by The qTox Project Contributors

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

#pragma once

#include <memory>

class RawDatabase;
namespace DbUpgrader
{
    bool dbSchemaUpgrade(std::shared_ptr<RawDatabase>& db);

    bool createCurrentSchema(RawDatabase& db);
    bool isNewDb(std::shared_ptr<RawDatabase>& db, bool& success);
    bool dbSchema0to1(RawDatabase& db);
    bool dbSchema1to2(RawDatabase& db);
    bool dbSchema2to3(RawDatabase& db);
    bool dbSchema3to4(RawDatabase& db);
    bool dbSchema4to5(RawDatabase& db);
    bool dbSchema5to6(RawDatabase& db);
    bool dbSchema6to7(RawDatabase& db);
    bool dbSchema7to8(RawDatabase& db);
    bool dbSchema8to9(RawDatabase& db);
    bool dbSchema9to10(RawDatabase& db);
    // 10to11 from DbTo11::dbSchema10to11
}
