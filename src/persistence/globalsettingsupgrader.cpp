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

#include "globalsettingsupgrader.h"
#include "src/persistence/settings.h"
#include "src/persistence/paths.h"

#include <QDebug>
#include <QFile>

namespace {
    bool version0to1(Settings& settings)
    {
        const auto& paths = settings.getPaths();

        QFile badFile{paths.getUserNodesFilePath()};
        if (badFile.exists()) {
            if (!badFile.rename(paths.getBackupUserNodesFilePath())) {
                qCritical() << "Failed to write to" << paths.getBackupUserNodesFilePath() << \
                    "aborting bootstrap node upgrade.";
                return false;
            }
        }
        qWarning() << "Moved" << badFile.fileName() << "to" << paths.getBackupUserNodesFilePath() << \
            "to mitigate a bug. Resetting bootstrap nodes to default.";
        return true;
    }
} // namespace

#include <cassert>

bool GlobalSettingsUpgrader::doUpgrade(Settings& settings, int fromVer, int toVer)
{
    if (fromVer == toVer) {
        return true;
    }

    if (fromVer > toVer) {
        qWarning().nospace() << "Settings version (" << fromVer
                             << ") is newer than we currently support (" << toVer
                             << "). Please upgrade qTox";
        return false;
    }

    using SettingsUpgradeFn = bool (*)(Settings&);
    std::vector<SettingsUpgradeFn> upgradeFns = {version0to1};

    assert(fromVer < static_cast<int>(upgradeFns.size()));
    assert(toVer == static_cast<int>(upgradeFns.size()));

    for (int i = fromVer; i < static_cast<int>(upgradeFns.size()); ++i) {
        auto const newSettingsVersion = i + 1;
        if (!upgradeFns[i](settings)) {
            qCritical() << "Failed to upgrade settings to version" << newSettingsVersion << "aborting";
            return false;
        }
        qDebug() << "Settings upgraded incrementally to schema version " << newSettingsVersion;
    }

    qInfo() << "Settings upgrade finished (settingsVersion" << fromVer << "->"
            << toVer << ")";
    return true;
}
