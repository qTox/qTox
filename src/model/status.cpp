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

namespace Status
{
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

    QString getIconPath(Status status, bool event)
    {
        const QString eventSuffix = event ? QStringLiteral("_notification") : QString();

        switch (status) {
        case Status::Online:
            return ":/img/status/online" + eventSuffix + ".svg";
        case Status::Away:
            return ":/img/status/away" + eventSuffix + ".svg";
        case Status::Busy:
            return ":/img/status/busy" + eventSuffix + ".svg";
        case Status::Offline:
            return ":/img/status/offline" + eventSuffix + ".svg";
        case Status::Blocked:
            return ":/img/status/blocked.svg";
        }
        qWarning() << "Status unknown";
        assert(false);
        return QString{};
    }
} // namespace Status
