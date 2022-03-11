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

#include "src/chatlog/chatlinestorage.h"
#include <QTest>

namespace
{
    class IdxChatLine : public ChatLine
    {
    public:
        explicit IdxChatLine(ChatLogIdx idx_)
            : ChatLine()
            , idx(idx_)
        {}

        ChatLogIdx get() { return idx; }
    private:
        ChatLogIdx idx;

    };

    class TimestampChatLine : public ChatLine
    {
    public:
        explicit TimestampChatLine(QDateTime dateTime)
            : ChatLine()
            , timestamp(dateTime)
        {}

        QDateTime get() { return timestamp; }
    private:
        QDateTime timestamp;
    };

    ChatLogIdx idxFromChatLine(ChatLine::Ptr p) {
        return std::static_pointer_cast<IdxChatLine>(p)->get();
    }

    QDateTime timestampFromChatLine(ChatLine::Ptr p) {
        return std::static_pointer_cast<TimestampChatLine>(p)->get();
    }

} // namespace


class TestChatLineStorage : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void testChatLogIdxAccess();
    void testIndexAccess();
    void testRangeBasedIteration();
    void testAppendingItems();
    void testPrependingItems();
    void testMiddleInsertion();
    void testIndexRemoval();
    void testItRemoval();
    void testDateLineAddition();
    void testDateLineRemoval();
    void testInsertionBeforeDates();
    void testInsertionAfterDate();
    void testContainsTimestamp();
    void testContainsIdx();
    void testEndOfStorageDateRemoval();
    void testConsecutiveDateLineRemoval();
private:
    ChatLineStorage storage;

    static constexpr size_t initialStartIdx = 10;
    static constexpr size_t initialEndIdx = 20;
    static const QDateTime initialTimestamp;

};

constexpr size_t TestChatLineStorage::initialStartIdx;
constexpr size_t TestChatLineStorage::initialEndIdx;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
const QDateTime TestChatLineStorage::initialTimestamp = QDate(2021, 01, 01).startOfDay();
#else
const QDateTime TestChatLineStorage::initialTimestamp(QDate(2021, 01, 01));
#endif

void TestChatLineStorage::init()
{
    storage = ChatLineStorage();

    for (auto idx = ChatLogIdx(initialStartIdx); idx < ChatLogIdx(initialEndIdx); ++idx) {
        storage.insertChatMessage(idx, initialTimestamp, std::make_shared<IdxChatLine>(idx));
    }
}

void TestChatLineStorage::testChatLogIdxAccess()
{
    for (auto idx = ChatLogIdx(initialStartIdx); idx < ChatLogIdx(initialEndIdx); ++idx) {
        QCOMPARE(idxFromChatLine(storage[idx]).get(), idx.get());
    }
}

void TestChatLineStorage::testIndexAccess()
{
    for (size_t i = 0; i < initialEndIdx - initialStartIdx; ++i) {
        QCOMPARE(idxFromChatLine(storage[i]).get(), initialStartIdx + i);
    }
}


void TestChatLineStorage::testRangeBasedIteration()
{
    auto idx = ChatLogIdx(initialStartIdx);

    for (const auto& p : storage) {
        QCOMPARE(idxFromChatLine(p).get(), idx.get());
        idx = idx + 1;
    }
}

void TestChatLineStorage::testAppendingItems()
{
    for (auto idx = ChatLogIdx(initialEndIdx); idx < ChatLogIdx(initialEndIdx + 10); ++idx) {
        storage.insertChatMessage(idx, initialTimestamp, std::make_shared<IdxChatLine>(idx));
        QCOMPARE(storage.lastIdx().get(), idx.get());
    }

    for (auto idx = ChatLogIdx(initialEndIdx); idx < storage.lastIdx(); ++idx) {
        QCOMPARE(idxFromChatLine(storage[idx]).get(), idx.get());
        QCOMPARE(idxFromChatLine(storage[idx.get() - initialStartIdx]).get(), idx.get());
    }
}

void TestChatLineStorage::testMiddleInsertion()
{
    ChatLogIdx newEnd = ChatLogIdx(initialEndIdx + 5);
    ChatLogIdx insertIdx = ChatLogIdx(initialEndIdx + 3);

    storage.insertChatMessage(newEnd, initialTimestamp, std::make_shared<IdxChatLine>(newEnd));
    storage.insertChatMessage(insertIdx, initialTimestamp, std::make_shared<IdxChatLine>(insertIdx));

    QCOMPARE(idxFromChatLine(storage[insertIdx]).get(), insertIdx.get());
    QCOMPARE(idxFromChatLine(storage[initialEndIdx - initialStartIdx]).get(), insertIdx.get());
    QCOMPARE(idxFromChatLine(storage[initialEndIdx - initialStartIdx + 1]).get(), newEnd.get());
}

void TestChatLineStorage::testPrependingItems()
{
    for (auto idx = ChatLogIdx(initialStartIdx - 1); idx != ChatLogIdx(-1); idx = idx - 1) {
        storage.insertChatMessage(idx, initialTimestamp, std::make_shared<IdxChatLine>(idx));
        QCOMPARE(storage.firstIdx().get(), idx.get());
    }

    for (auto idx = storage.firstIdx(); idx < storage.lastIdx(); ++idx) {
        QCOMPARE(idxFromChatLine(storage[idx]).get(), idx.get());
        QCOMPARE(idxFromChatLine(storage[idx.get()]).get(), idx.get());
    }
}

void TestChatLineStorage::testIndexRemoval()
{
    QCOMPARE(initialStartIdx, static_cast<size_t>(10));
    QCOMPARE(initialEndIdx, static_cast<size_t>(20));
    QCOMPARE(storage.size(), static_cast<size_t>(10));

    storage.erase(ChatLogIdx(11));

    QCOMPARE(storage.size(), static_cast<size_t>(9));

    QCOMPARE(idxFromChatLine(storage[0]).get(), static_cast<size_t>(10));
    QCOMPARE(idxFromChatLine(storage[1]).get(), static_cast<size_t>(12));

    auto idx = static_cast<size_t>(12);
    for (auto it = std::next(storage.begin()); it != storage.end(); ++it) {
        QCOMPARE(idxFromChatLine((*it)).get(), idx++);
    }
}

void TestChatLineStorage::testItRemoval()
{
    auto it = storage.begin();
    it = it + 2;

    storage.erase(it);

    QCOMPARE(idxFromChatLine(storage[0]).get(), initialStartIdx);
    QCOMPARE(idxFromChatLine(storage[1]).get(), initialStartIdx + 1);
    // Item should have been removed
    QCOMPARE(idxFromChatLine(storage[2]).get(), initialStartIdx + 3);
}

void TestChatLineStorage::testDateLineAddition()
{
    storage.insertDateLine(initialTimestamp, std::make_shared<TimestampChatLine>(initialTimestamp));
    auto newTimestamp = initialTimestamp.addDays(1);
    storage.insertDateLine(newTimestamp, std::make_shared<TimestampChatLine>(newTimestamp));

    QCOMPARE(storage.size(), initialEndIdx - initialStartIdx + 2);
    QCOMPARE(timestampFromChatLine(storage[0]), initialTimestamp);
    QCOMPARE(timestampFromChatLine(storage[storage.size() - 1]), newTimestamp);

    for (size_t i = 1; i < storage.size() - 2; ++i)
    {
        // Ensure that indexed items all stayed in the right order
        QCOMPARE(idxFromChatLine(storage[i]).get(), idxFromChatLine(storage[ChatLogIdx(initialStartIdx + i - 1)]).get());
    }

}

void TestChatLineStorage::testDateLineRemoval()
{
    // For the time being there is no removal requirement
    storage.insertDateLine(initialTimestamp, std::make_shared<TimestampChatLine>(initialTimestamp));

    QVERIFY(storage.contains(initialTimestamp));
    QCOMPARE(timestampFromChatLine(storage[0]), initialTimestamp);

    storage.erase(storage.begin());

    QVERIFY(!storage.contains(initialTimestamp));
    QCOMPARE(idxFromChatLine(storage[0]).get(), initialStartIdx);
}

void TestChatLineStorage::testInsertionBeforeDates()
{
    storage.insertDateLine(initialTimestamp, std::make_shared<TimestampChatLine>(initialTimestamp));

    auto yesterday = initialTimestamp.addDays(-1);
    storage.insertDateLine(yesterday, std::make_shared<TimestampChatLine>(yesterday));

    auto firstIdx = ChatLogIdx(initialStartIdx - 2);
    storage.insertChatMessage(firstIdx, initialTimestamp.addDays(-2), std::make_shared<IdxChatLine>(firstIdx));

    QCOMPARE(idxFromChatLine(storage[0]).get(), firstIdx.get());
    QCOMPARE(timestampFromChatLine(storage[1]), yesterday);
    QCOMPARE(timestampFromChatLine(storage[2]), initialTimestamp);
    QCOMPARE(idxFromChatLine(storage[3]).get(), initialStartIdx);

    auto secondIdx = ChatLogIdx(initialStartIdx - 1);
    storage.insertChatMessage(secondIdx, initialTimestamp.addDays(-1), std::make_shared<IdxChatLine>(secondIdx));

    QCOMPARE(idxFromChatLine(storage[0]).get(), firstIdx.get());
    QCOMPARE(timestampFromChatLine(storage[1]), yesterday);
    QCOMPARE(idxFromChatLine(storage[2]).get(), secondIdx.get());
    QCOMPARE(timestampFromChatLine(storage[3]), initialTimestamp);
    QCOMPARE(idxFromChatLine(storage[4]).get(), initialStartIdx);
}

void TestChatLineStorage::testInsertionAfterDate()
{
    auto newTimestamp = initialTimestamp.addDays(1);
    storage.insertDateLine(newTimestamp, std::make_shared<TimestampChatLine>(newTimestamp));

    QCOMPARE(storage.size(), initialEndIdx - initialStartIdx + 1);
    QCOMPARE(timestampFromChatLine(storage[initialEndIdx - initialStartIdx]), newTimestamp);

    storage.insertChatMessage(ChatLogIdx(initialEndIdx), newTimestamp, std::make_shared<IdxChatLine>(ChatLogIdx(initialEndIdx)));
    QCOMPARE(idxFromChatLine(storage[initialEndIdx - initialStartIdx + 1]).get(), initialEndIdx);
    QCOMPARE(idxFromChatLine(storage[ChatLogIdx(initialEndIdx)]).get(), initialEndIdx);
}

void TestChatLineStorage::testContainsTimestamp()
{
    QCOMPARE(storage.contains(initialTimestamp), false);
    storage.insertDateLine(initialTimestamp, std::make_shared<TimestampChatLine>(initialTimestamp));
    QCOMPARE(storage.contains(initialTimestamp), true);
}

void TestChatLineStorage::testContainsIdx()
{
    QCOMPARE(storage.contains(ChatLogIdx(initialEndIdx)), false);
    QCOMPARE(storage.contains(ChatLogIdx(initialStartIdx)), true);
}

void TestChatLineStorage::testEndOfStorageDateRemoval()
{
    auto tomorrow = initialTimestamp.addDays(1);
    storage.insertDateLine(tomorrow, std::make_shared<TimestampChatLine>(tomorrow));
    storage.insertChatMessage(ChatLogIdx(initialEndIdx), tomorrow, std::make_shared<IdxChatLine>(ChatLogIdx(initialEndIdx)));

    QCOMPARE(storage.size(), initialEndIdx - initialStartIdx + 2);

    auto it = storage.begin() + storage.size() - 2;
    QCOMPARE(timestampFromChatLine(*it++), tomorrow);
    QCOMPARE(idxFromChatLine(*it).get(), initialEndIdx);

    storage.erase(it);

    QCOMPARE(storage.size(), initialEndIdx - initialStartIdx);

    it = storage.begin() + storage.size() - 1;
    QCOMPARE(idxFromChatLine(*it++).get(), initialEndIdx - 1);
}

void TestChatLineStorage::testConsecutiveDateLineRemoval()
{
    auto todayPlus1 = initialTimestamp.addDays(1);
    auto todayPlus2 = initialTimestamp.addDays(2);

    auto todayPlus1Idx = ChatLogIdx(initialEndIdx);
    auto todayPlus1Idx2 = ChatLogIdx(initialEndIdx + 1);
    auto todayPlus2Idx = ChatLogIdx(initialEndIdx + 2);


    storage.insertDateLine(todayPlus1, std::make_shared<TimestampChatLine>(todayPlus1));
    storage.insertChatMessage(todayPlus1Idx, todayPlus1, std::make_shared<IdxChatLine>(todayPlus1Idx));
    storage.insertChatMessage(todayPlus1Idx2, todayPlus1, std::make_shared<IdxChatLine>(todayPlus1Idx2));

    storage.insertDateLine(todayPlus2, std::make_shared<TimestampChatLine>(todayPlus2));
    storage.insertChatMessage(todayPlus2Idx, todayPlus2, std::make_shared<IdxChatLine>(todayPlus2Idx));

    // 2 date lines and 3 messages were inserted for a total of 5 new lines
    QCOMPARE(storage.size(), initialEndIdx - initialStartIdx + 5);

    storage.erase(storage.find(todayPlus1Idx2));

    auto newItemIdxStart = initialEndIdx - initialStartIdx;

    // Only the chat message should have been removed
    QCOMPARE(storage.size(), initialEndIdx - initialStartIdx + 4);

    QCOMPARE(timestampFromChatLine(storage[newItemIdxStart]), todayPlus1);
    QCOMPARE(idxFromChatLine(storage[newItemIdxStart + 1]).get(), todayPlus1Idx.get());
    QCOMPARE(timestampFromChatLine(storage[newItemIdxStart + 2]), todayPlus2);
    QCOMPARE(idxFromChatLine(storage[newItemIdxStart + 3]).get(), todayPlus2Idx.get());

    storage.erase(storage.find(todayPlus1Idx));

    // The chat message + the dateline for it should have been removed as there
    // were 2 adjacent datelines caused by the removal
    QCOMPARE(storage.size(), initialEndIdx - initialStartIdx + 2);
    QCOMPARE(timestampFromChatLine(storage[newItemIdxStart]), todayPlus2);
    QCOMPARE(idxFromChatLine(storage[newItemIdxStart + 1]).get(), todayPlus2Idx.get());
}

QTEST_GUILESS_MAIN(TestChatLineStorage)
#include "chatlinestorage_test.moc"
