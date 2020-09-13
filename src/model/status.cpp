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

#include <src/model/status.h>

#include <QString>
#include <QPixmap>
#include <QDebug>
#include <QObject>

#include <cassert>

namespace Status {
    QString getTitle(Status status)
    {
        switch (status) {
        case Status::Online:
            return QObject::tr("online", "contact status");
        case Status::Away:
            return QObject::tr("away", "contact status");
        case Status::Busy:
            return QObject::tr("busy", "contact status");
        case Status::Offline:
            return QObject::tr("offline", "contact status");
        case Status::Blocked:
            return QObject::tr("blocked", "contact status");
        }

        assert(false);
        return QStringLiteral("");
    }

    QString getAssetSuffix(Status status)
    {
        switch (status) {
        case Status::Online:
            return "online";
        case Status::Away:
            return "away";
        case Status::Busy:
            return "busy";
        case Status::Offline:
            return "offline";
        case Status::Blocked:
            return "blocked";
        }
        assert(false);
        return QStringLiteral("");
    }

    QString getIconPath(Status status, bool event)
    {
        const QString eventSuffix = event ? QStringLiteral("_notification") : QString();
        const QString statusSuffix = getAssetSuffix(status);
        if (status == Status::Blocked) {
            return ":/img/status/" + statusSuffix + ".svg";
        } else {
            return ":/img/status/" + statusSuffix + eventSuffix + ".svg";
        }
    }

    bool isOnline(Status status)
    {
        return status != Status::Offline && status != Status::Blocked;
    }

    bool toToxStatus(IToxStatus::ToxStatus &toxStatus, Status status)
    {
        switch (status) {
        case Status::Online:
            toxStatus = IToxStatus::ToxStatus::Online;
            return true;
            break;

        case Status::Away:
            toxStatus = IToxStatus::ToxStatus::Away;
            return true;
            break;

        case Status::Busy:
            toxStatus = IToxStatus::ToxStatus::Busy;
            return true;
            break;
        case Status::Offline:
            toxStatus = IToxStatus::ToxStatus::Offline;
            return true;
            break;

        default:
            return false;
            break;
        }
    }

    bool fromToxStatus(Status &status, IToxStatus::ToxStatus toxStatus)
    {
        switch (toxStatus) {
        case IToxStatus::ToxStatus::Online:
            status = Status::Online;
            return true;
            break;
        case IToxStatus::ToxStatus::Offline:
            status = Status::Offline;
            return true;
            break;

        case IToxStatus::ToxStatus::Busy:
            status = Status::Busy;
            return true;
            break;

        case IToxStatus::ToxStatus::Away:
            status = Status::Away;
            return true;
            break;

        default:
            return false;
            break;
        }
    }
} // namespace Status
