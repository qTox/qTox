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
    FilterSearch filter;
    PeriodSearch period;
    QDate date;
    bool isUpdate;

    ParameterSearch() {
        filter = FilterSearch::None;
        period = PeriodSearch::None;
        isUpdate = false;
    }

    bool operator ==(const ParameterSearch& other) {
        if (this->filter != other.filter) {
            return false;
        }

        if (this->period != other.period) {
            return false;
        }

        if (this->date != other.date) {
            return false;
        }

        return true;
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
