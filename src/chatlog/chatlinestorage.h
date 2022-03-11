/*
    Copyright © 2021 by The qTox Project Contributors

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

#include "src/chatlog/chatline.h"
#include "src/model/ichatlog.h"

#include <QDateTime>

#include <vector>
#include <map>

/**
 * Helper class to keep track of what we're currently rendering and in what order
 * Some constraints that may not be obvious
 *   * Rendered views are not always contiguous. When we clear the chatlog
 *   ongoing file transfers are not removed or else we would have no way to stop
 *   them. If history is loaded after this point then we could actually be
 *   inserting elements both before and in the middle of our existing rendered
 *   items
 *   * We need to be able to go from ChatLogIdx to rendered row index. E.g. if
 *   an SQL query is made for search, we need to map the result back to the
 *   displayed row to send it
 *   * We need to be able to map rows back to ChatLogIdx in order to decide where
 *   to insert newly added messages
 *   * Not all rendered lines will have an associated ChatLogIdx, date lines for
 *   example are not clearly at any ChatLogIdx
 *   * Need to track date messages to ensure that if messages are inserted above
 *   the current position the date line is moved appropriately
 *
 * The class is designed to be used like a vector over the currently rendered
 * items, but with some tweaks for ensuring items tied to the current view are
 * moved correctly (selection indexes, removal of associated date lines,
 * mappings of ChatLogIdx -> ChatLine::Ptr, etc.)
 */
class ChatLineStorage
{

    struct IdxInfo
    {
        size_t linePos;
        QDateTime timestamp;
    };
    using Lines_t = std::vector<ChatLine::Ptr>;
    using DateLineMap_t = std::map<ChatLine::Ptr, QDateTime>;
    using IdxInfoMap_t = std::map<ChatLogIdx, IdxInfo>;

public:
    // Types to conform with other containers
    using size_type = Lines_t::size_type;
    using reference = Lines_t::reference;
    using const_reference = Lines_t::const_reference;
    using const_iterator = Lines_t::const_iterator;
    using iterator = Lines_t::iterator;


public:
    iterator insertChatMessage(ChatLogIdx idx, QDateTime timestamp, ChatLine::Ptr line);
    iterator insertDateLine(QDateTime timestamp, ChatLine::Ptr line);

    ChatLogIdx firstIdx() const { return idxInfoMap.begin()->first; }

    ChatLogIdx lastIdx() const { return idxInfoMap.rbegin()->first; }

    bool contains(ChatLogIdx idx) const { return idxInfoMap.find(idx) != idxInfoMap.end(); }

    bool contains(QDateTime timestamp) const;

    iterator find(ChatLogIdx idx);
    iterator find(ChatLine::Ptr line);

    const_reference operator[](size_type idx) const { return lines[idx]; }

    const_reference operator[](ChatLogIdx idx) const { return lines[idxInfoMap.at(idx).linePos]; }

    size_type size() const { return lines.size(); }

    iterator begin() { return lines.begin(); }
    iterator end() { return lines.end(); }

    bool empty() const { return lines.empty(); }

    bool hasIndexedMessage() const { return !idxInfoMap.empty(); }

    void clear()
    {
        idxInfoMap.clear();
        dateMap.clear();
        return lines.clear();
    }

    reference front() { return lines.front(); }
    reference back() { return lines.back(); }

    void erase(ChatLogIdx idx);
    iterator erase(iterator it);

private:
    iterator equivalentLineIterator(IdxInfoMap_t::iterator it);

    IdxInfoMap_t::iterator equivalentInfoIterator(iterator it);

    IdxInfoMap_t::iterator infoIteratorForIdx(ChatLogIdx idx_);

    iterator adjustItForDate(iterator it, QDateTime timestamp);

    void incrementLinePosAfter(IdxInfoMap_t::iterator it);
    void decrementLinePosAfter(IdxInfoMap_t::iterator it);
    bool shouldRemovePreviousLine(iterator prevIt, iterator it);

    std::vector<ChatLine::Ptr> lines;
    std::map<ChatLine::Ptr, QDateTime> dateMap;
    IdxInfoMap_t idxInfoMap;
};
