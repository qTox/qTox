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

#pragma once

#include <QDate>
#include <QRegularExpression>

enum class FilterSearch {
    None,
    Register,
    WordsOnly,
    Regular,
    RegisterAndWordsOnly,
    RegisterAndRegular
};

enum class PeriodSearch {
    None,
    WithTheEnd,
    WithTheFirst,
    AfterDate,
    BeforeDate
};

enum class SearchDirection {
    Up,
    Down
};

struct ParameterSearch {
    FilterSearch filter{FilterSearch::None};
    PeriodSearch period{PeriodSearch::None};
    QDateTime time;
    bool isUpdate{false};

    bool operator ==(const ParameterSearch& other) {
        return filter == other.filter &&
            period == other.period &&
            time == other.time;
    }

    bool operator !=(const ParameterSearch& other) {
        return !(*this == other);
    }
};

class SearchExtraFunctions {
public:
    /**
     * @brief generateFilterWordsOnly generate string for filter "Whole words only" for correct search phrase
     * containing symbols "\[]/^$.|?*+(){}"
     * @param phrase for search
     * @return new phrase for search
     */
    static QString generateFilterWordsOnly(const QString &phrase) {
        QString filter = QRegularExpression::escape(phrase);

        const QString symbols = QStringLiteral("\\[]/^$.|?*+(){}");

        if (filter != phrase) {
            if (filter.left(1) != QLatin1String("\\")) {
                filter = QLatin1String("\\b") + filter;
            } else {
                filter = QLatin1String("(^|\\s)") + filter;
            }
            if (!symbols.contains(filter.right(1))) {
                filter += QLatin1String("\\b");
            } else {
                filter += QLatin1String("($|\\s)");
            }
        } else {
            filter = QStringLiteral("\\b%1\\b").arg(filter);
        }

        return filter;
    }
};
