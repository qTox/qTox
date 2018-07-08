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
    static QString generateFilterWordsOnly(const QString &phrase) {
        QString filter = phrase;

        if (filter.contains("\\")) {
            filter.replace("\\", "\\\\");

            if (filter.front() != '\\') {
                filter = "\\b" + filter;
            }
            if (filter.back() != '\\') {
                filter += "\\b";
            }
        } else {
            filter = QStringLiteral("\\b%1\\b").arg(filter);
        }

        return filter;
    };
};

#endif //SEARCHTYPES_H
