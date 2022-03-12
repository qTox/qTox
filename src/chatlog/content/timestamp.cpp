/*
    Copyright © 2015-2019 by The qTox Project Contributors

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

#include "timestamp.h"

Timestamp::Timestamp(const QDateTime& time_, const QString& format,
    const QFont& font, DocumentCache& documentCache_, Settings& settings_,
    Style& style_)
    : Text(documentCache_, settings_, style_, time_.toString(format), font, false,
        time_.toString(format))
{
    time = time_;
}

QDateTime Timestamp::getTime()
{
    return time;
}

QSizeF Timestamp::idealSize()
{
    if (doc) {
        return QSizeF(qMin(doc->idealWidth(), width), doc->size().height());
    }
    return size;
}
