#ifndef SEARCHTYPES_H
#define SEARCHTYPES_H

#include <QDate>

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

struct ParameterSearch {
    FilterSearch filter{FilterSearch::None};
    PeriodSearch period{PeriodSearch::None};
    QDate date;
    bool isUpdate{false};

    bool operator ==(const ParameterSearch& other) {
        return filter == other.filter &&
            period == other.period &&
            date == other.date;
    }

    bool operator !=(const ParameterSearch& other) {
        return !(*this == other);
    }
};

class SearchExtraFunctions {
public:
    /**
     * @brief generateFilterWordsOnly generate string for filter "Whole words only" for correct search phrase
     * containing symbols "\"
     * @param phrase for search
     * @return new phrase for search
     */
    static QString generateFilterWordsOnly(const QString &phrase) {
        QString filter = phrase;

        if (filter.contains("\\")) {
            filter.replace("\\", "\\\\");

            if (filter.left(1) != "\\") {
                filter = "\\b" + filter;
            }
            if (filter.right(1) != "\\") {
                filter += "\\b";
            }
        } else {
            filter = QStringLiteral("\\b%1\\b").arg(filter);
        }

        return filter;
    }
};

#endif //SEARCHTYPES_H
