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

#include "src/persistence/db/rawdatabase.h"

#include <QVector>

class ToxPk;

namespace DbTo11
{
    bool dbSchema10to11(RawDatabase& db);
namespace PeersToAuthors
{
    bool appendPeersToAuthorsQueries(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries);
    void appendCreateNewTablesQueries(QVector<RawDatabase::Query>& upgradeQueries);
    bool appendPopulateAuthorQueries(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries);
    bool appendUpdateAliasesFkQueries(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries);
    void appendReplaceOldTablesQueries(QVector<RawDatabase::Query>& upgradeQueries);
} // namespace PeersToAuthors

namespace PeersToChats
{
    bool appendPeersToChatsQueries(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries);
    void appendCreateNewTablesQueries(QVector<RawDatabase::Query>& upgradeQueries);
    bool appendPopulateChatsQueries(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries);
    bool appendUpdateHistoryFkQueries(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries);
    void appendReplaceOldTablesQueries(QVector<RawDatabase::Query>& upgradeQueries);
    void appendUpdateSystemMessagesFkQueries(QVector<RawDatabase::Query>& upgradeQueries);
    void appendUpdateBrokenMessagesFkQueries(QVector<RawDatabase::Query>& upgradeQueries);
    void appendUpdateFauxOfflinePendingFkQueries(QVector<RawDatabase::Query>& upgradeQueries);
} // namespace PeersToChats

    void appendUpdateTextMessagesFkQueries(QVector<RawDatabase::Query>& upgradeQueries);
    void appendUpdateFileTransfersFkQueries(QVector<RawDatabase::Query>& upgradeQueries);
    void appendDropPeersQueries(QVector<RawDatabase::Query>& upgradeQueries);
} // namespace DbTo11
