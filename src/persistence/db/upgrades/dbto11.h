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
    bool peersToAuthors(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries);
    void createNewTables(QVector<RawDatabase::Query>& upgradeQueries);
    bool populateAuthors(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries);
    bool updateAliasesFk(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries);
    void replaceOldTables(QVector<RawDatabase::Query>& upgradeQueries);
} // namespace PeersToAuthors

namespace PeersToChats
{
    bool peersToChats(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries);
    void createNewTables(QVector<RawDatabase::Query>& upgradeQueries);
    bool populateChats(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries);
    bool updateHistoryFk(RawDatabase& db, QVector<RawDatabase::Query>& upgradeQueries);
    void replaceOldTables(QVector<RawDatabase::Query>& upgradeQueries);
    void updateSystemMessagesFk(QVector<RawDatabase::Query>& upgradeQueries);
    void updateBrokenMessagesFk(QVector<RawDatabase::Query>& upgradeQueries);
    void updateFauxOfflinePendingFk(QVector<RawDatabase::Query>& upgradeQueries);
} // namespace PeersToChats

    void updateTextMessagesFk(QVector<RawDatabase::Query>& upgradeQueries);
    void updateFileTransfersFk(QVector<RawDatabase::Query>& upgradeQueries);
    void dropPeers(QVector<RawDatabase::Query>& upgradeQueries);
} // namespace DbTo11
