/*
    Copyright © 2020-2021 by The qTox Project Contributors

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

#include "chatlinestorage.h"

#include <QDebug>

ChatLineStorage::iterator ChatLineStorage::insertChatMessage(ChatLogIdx idx, QDateTime timestamp, ChatLine::Ptr line)
{
    if (idxInfoMap.find(idx) != idxInfoMap.end()) {
        qWarning() << "Index is already rendered, not updating";
        return lines.end();
    }

    auto linePosIncrementIt = infoIteratorForIdx(idx);

    auto insertionPoint = equivalentLineIterator(linePosIncrementIt);
    insertionPoint = adjustItForDate(insertionPoint, timestamp);

    insertionPoint = lines.insert(insertionPoint, line);

    // All indexes after the insertion have to be incremented by one
    incrementLinePosAfter(linePosIncrementIt);

    // Newly inserted index is insertinPoint - start
    IdxInfo info;
    info.linePos = std::distance(lines.begin(), insertionPoint);
    info.timestamp = timestamp;
    idxInfoMap[idx] = info;

    return insertionPoint;
}

ChatLineStorage::iterator ChatLineStorage::insertDateLine(QDateTime timestamp, ChatLine::Ptr line)
{
    // Assume we only need to render one date line per date. I.e. this does
    // not handle the case of
    // * Message inserted Jan 3
    // * Message inserted Jan 4
    // * Message inserted Jan 3
    // In this case the second "Jan 3" message will appear to have been sent
    // on Jan 4
    // As of right now this should not be a problem since all items should
    // be sent/received in order. If we ever implement sender timestamps and
    // the sender screws us by changing their time we may need to revisit this
    auto idxMapIt = std::find_if(idxInfoMap.begin(), idxInfoMap.end(), [&] (const IdxInfoMap_t::value_type& v) {
        return timestamp <= v.second.timestamp;
    });

    auto insertionPoint = equivalentLineIterator(idxMapIt);
    insertionPoint = adjustItForDate(insertionPoint, timestamp);

    insertionPoint = lines.insert(insertionPoint, line);

    // All indexes after the insertion have to be incremented by one
    incrementLinePosAfter(idxMapIt);

    dateMap[line] = timestamp;

    return insertionPoint;
}

bool ChatLineStorage::contains(QDateTime timestamp) const
{
    auto it =  std::find_if(dateMap.begin(), dateMap.end(), [&] (DateLineMap_t::value_type v) {
        return v.second == timestamp;
    });

    return it != dateMap.end();
}

ChatLineStorage::iterator ChatLineStorage::find(ChatLogIdx idx)
{
    auto infoIt = infoIteratorForIdx(idx);
    if (infoIt == idxInfoMap.end()) {
        return lines.end();
    }

    return lines.begin() + infoIt->second.linePos;

}

ChatLineStorage::iterator ChatLineStorage::find(ChatLine::Ptr line)
{
    return std::find(lines.begin(), lines.end(), line);
}

void ChatLineStorage::erase(ChatLogIdx idx)
{
    auto linePosDecrementIt = infoIteratorForIdx(idx);
    auto lineIt = equivalentLineIterator(linePosDecrementIt);

    erase(lineIt);
}

ChatLineStorage::iterator ChatLineStorage::erase(iterator it)
{
    iterator prevIt = it;

    do {
        it = prevIt;

        auto infoIterator = equivalentInfoIterator(it);
        auto dateMapIt = dateMap.find(*it);

        if (dateMapIt != dateMap.end()) {
            dateMap.erase(dateMapIt);
        }

        if (infoIterator != idxInfoMap.end()) {
            infoIterator = idxInfoMap.erase(infoIterator);
            decrementLinePosAfter(infoIterator);
        }

        it = lines.erase(it);

        if (it > lines.begin()) {
            prevIt = std::prev(it);
        } else {
            prevIt = lines.end();
        }
    } while (shouldRemovePreviousLine(prevIt, it));

    return it;
}

ChatLineStorage::iterator ChatLineStorage::equivalentLineIterator(IdxInfoMap_t::iterator it)
{
    if (it == idxInfoMap.end()) {
        return lines.end();
    }

    return std::next(lines.begin(), it->second.linePos);
}

ChatLineStorage::IdxInfoMap_t::iterator ChatLineStorage::equivalentInfoIterator(iterator it)
{
    auto idx = static_cast<size_t>(std::distance(lines.begin(), it));
    auto equivalentIt = std::find_if(idxInfoMap.begin(), idxInfoMap.end(), [&](const IdxInfoMap_t::value_type& v) {
        return v.second.linePos >= idx;
    });

    return equivalentIt;
}

ChatLineStorage::IdxInfoMap_t::iterator ChatLineStorage::infoIteratorForIdx(ChatLogIdx idx_)
{
    // If lower_bound proves to be expensive for appending we can try
    // special casing when idx > idxToLineMap.rbegin()->first

    // If we find an exact match we return that index, otherwise we return
    // the first item after it. It's up to the caller to check if there's an
    // exact match first
    auto it = std::lower_bound(idxInfoMap.begin(), idxInfoMap.end(), idx_, [](const IdxInfoMap_t::value_type& v, ChatLogIdx idx) {
        return v.first < idx;
    });

    return it;
}

ChatLineStorage::iterator ChatLineStorage::adjustItForDate(iterator it, QDateTime timestamp)
{
    // Continuously move back until either
    // 1. The dateline found is earlier than our timestamp
    // 2. There are no more datelines
    while (it > lines.begin()) {
        auto possibleDateIt = it - 1;
        auto dateIt = dateMap.find(*possibleDateIt);
        if (dateIt == dateMap.end()) {
            break;
        }

        if (dateIt->second > timestamp) {
            it = possibleDateIt;
        } else {
            break;
        }
    }

    return it;
}

void ChatLineStorage::incrementLinePosAfter(IdxInfoMap_t::iterator inputIt)
{
    for (auto it = inputIt; it != idxInfoMap.end(); ++it) {
        it->second.linePos++;
    }
}

void ChatLineStorage::decrementLinePosAfter(IdxInfoMap_t::iterator inputIt)
{
    // All indexes after the insertion have to be incremented by one
    for (auto it = inputIt; it != idxInfoMap.end(); ++it) {
        it->second.linePos--;
    }
}

bool ChatLineStorage::shouldRemovePreviousLine(iterator prevIt, iterator it)
{
    return prevIt != lines.end() && // Previous iterator is valid
        dateMap.find(*prevIt) != dateMap.end() && // Previous iterator is a date line
        (
            it == lines.end() || // Previous iterator is the last line
            dateMap.find(*it) != dateMap.end() // Adjacent date lines
        );
}
