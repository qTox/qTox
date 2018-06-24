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

#endif //SEARCHTYPES_H
