/*
    Copyright © 2017 by The qTox Project Contributors

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

#include "src/core/toxpk.h"
#include "src/friendlist.h"
#include "src/model/contacts/contactscircle.h"
#include "src/model/contacts/friend.h"

#include <QSignalSpy>
#include <QtTest/QtTest>

#define GET_PK(str) ToxPk{QByteArray::fromHex(QStringLiteral(str).toLatin1())}

static const ToxPk PUBLIC_KEYS[] {
    GET_PK("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D30"),
    GET_PK("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D31"),
    GET_PK("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D32"),
    GET_PK("C7719C6808C14B77348004956D1D98046CE09A34370E7608150EAD74C3815D33"),
};

#undef GET_PK

static const QMap<ToxPk, Friend*> ID_TO_FRIEND {
    { PUBLIC_KEYS[0], new Friend(0, PUBLIC_KEYS[0], "bbbb", "bbbb")},
    { PUBLIC_KEYS[1], new Friend(1, PUBLIC_KEYS[1], "BBAB", "slkjlsdkj")},
    { PUBLIC_KEYS[2], new Friend(2, PUBLIC_KEYS[2], "ldklk", "JKHAS asdkl")},
    { PUBLIC_KEYS[3], new Friend(3, PUBLIC_KEYS[3], "", "asasdlk ASLdkjslk")},
};

/**
 * @brief Checks that array of friends sorted by name in ascending order
 */
static bool isSorted(const std::vector<const Friend*> friends)
{
    const ContactsCircle::Comparator comp = ContactsCircle::getComparator();
    if (friends.empty()) {
        return true;
    }

    const Friend* current = friends[0];
    for (int i = 1; i < friends.size(); ++i) {
        const Friend* next = friends[i];
        if (!comp(current, next)) {
            return false;
        }

        current = next;
    }

    return true;
}

Q_DECLARE_METATYPE(ToxPk)

class TestContactsCircle : public QObject
{
    Q_OBJECT

public:
    TestContactsCircle()
    {
        // stub for ContactsCircle::friendSupplier function
        ContactsCircle::setFriendSupplier([this] (const ToxPk& pk)
        {
            return ID_TO_FRIEND.contains(pk) ? ID_TO_FRIEND[pk] : nullptr;
        });
    }

    ~TestContactsCircle()
    {
        for (const Friend* f: ID_TO_FRIEND) {
            delete f;
        }
    }

private slots:
    void initMetaSystem();

    void friendAddedSignalEmitted();
    void addFriend_FriendIsAdded();
    void addFriendTwice_FriendIsNotAdded();
    void addNonexistentFriend_FriendIsNotAdded();
    void addFriend_isSorted();

    void friendRemovedSignalEmitted();
    void removeFriend_FriendIsRemoved();
    void removeFriendTwice_FriendIsNotRemoved();
    void removeFriendNotPresentedInCircle_FriendIsNotRemoved();
    void removeFriend_isSorted();

    void hasFriend_FriendIsFound();
    void hasFriend_FriendIsNotFound();

    void expansionChanged_SignalEmitted();
    void expansionIsNotChanged_SignalIsNotEmitted();

    void circleRenamedSignalEmitted();
    void invalidCircleName_CircleIsNotRenamed();

private:
};

/**
 * @brief For some reason, first of tests friend(Added|Removed)SignalEmitted fails
 * This test is a full copy of friendAddedSignalEmitted test and purposed to make
 * some actions needed for Qt internals to work properly
 */
void TestContactsCircle::initMetaSystem()
{
    ContactsCircle circle(0, "test");
    QSignalSpy spy(&circle, SIGNAL(friendAdded(const ToxPk&)));
    const ToxPk friendPk = PUBLIC_KEYS[0];
    circle.addFriend(friendPk);
    // just performing cast to make Meta System work
    spy.takeFirst().at(0).value<ToxPk>();
}

/**
 * @brief Checks that ContactsCircle::friendAdded signal is emitting properly if Friend is valid
 */
void TestContactsCircle::friendAddedSignalEmitted()
{
    ContactsCircle circle(0, "test");
    const ToxPk friendPk = PUBLIC_KEYS[0];
    QSignalSpy spy(&circle, SIGNAL(friendAdded(const ToxPk&)));
    circle.addFriend(friendPk);
    // check that signal was emitted only once
    QCOMPARE(spy.count(), 1);
    // check that signal was emitted with proper parameter
    const QList<QVariant> signalArgs = spy.takeFirst();
    const ToxPk emittedFriendPk = signalArgs.at(0).value<ToxPk>();
    QCOMPARE(emittedFriendPk, friendPk);
}

/**
 * @brief Tests adding of a valid Friend
 */
void TestContactsCircle::addFriend_FriendIsAdded()
{
    ContactsCircle circle(0, "test");
    QVERIFY(circle.getFriends().empty());
    const ToxPk friendPk = PUBLIC_KEYS[0];
    circle.addFriend(friendPk);
    QCOMPARE(circle.getFriends().size(), 1ul);
}

void TestContactsCircle::addFriendTwice_FriendIsNotAdded()
{
    ContactsCircle circle(0, "test");
    QVERIFY(circle.getFriends().empty());
    const ToxPk friendPk = PUBLIC_KEYS[0];
    QSignalSpy spy(&circle, SIGNAL(friendAdded(const ToxPk&)));
    for (int i = 0; i < 2; ++i) {
        circle.addFriend(friendPk);
        QCOMPARE(circle.getFriends().size(), 1ul);
        QCOMPARE(spy.count(), 1);
    }
}

void TestContactsCircle::addNonexistentFriend_FriendIsNotAdded()
{
    ContactsCircle circle(0, "test");
    QVERIFY(circle.getFriends().empty());
    const ToxPk friendPk{};
    QSignalSpy spy(&circle, SIGNAL(friendAdded(const ToxPk&)));
    circle.addFriend(friendPk);
    QVERIFY(circle.getFriends().empty());
    QVERIFY(spy.isEmpty());
}

void TestContactsCircle::addFriend_isSorted()
{
    ContactsCircle circle(0, "test");
    for (const ToxPk pk: PUBLIC_KEYS) {
        circle.addFriend(pk);
        QVERIFY(isSorted(circle.getFriends()));
    }
}

void TestContactsCircle::friendRemovedSignalEmitted()
{
    ContactsCircle circle(0, "test");
    const ToxPk friendPk = PUBLIC_KEYS[0];
    QSignalSpy spy(&circle, SIGNAL(friendRemoved(const ToxPk&)));
    circle.addFriend(friendPk);
    circle.removeFriend(friendPk);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> signalArgs = spy.takeFirst();
    const ToxPk emittedFriendPk = signalArgs.at(0).value<ToxPk>();
    QCOMPARE(emittedFriendPk, friendPk);
}

void TestContactsCircle::removeFriend_FriendIsRemoved()
{
    ContactsCircle circle(0, "test");
    const ToxPk friendPk = PUBLIC_KEYS[0];
    circle.addFriend(friendPk);
    QCOMPARE(circle.getFriends().size(), 1ul);
    circle.removeFriend(friendPk);
    QVERIFY(circle.getFriends().empty());
}

void TestContactsCircle::removeFriendTwice_FriendIsNotRemoved()
{
    ContactsCircle circle(0, "test");
    QVERIFY(circle.getFriends().empty());
    const ToxPk friendPk = PUBLIC_KEYS[0];
    circle.addFriend(friendPk);
    QCOMPARE(circle.getFriends().size(), 1ul);
    QSignalSpy spy(&circle, SIGNAL(friendRemoved(const ToxPk&)));
    for (int i = 0; i < 2; ++i) {
        circle.removeFriend(friendPk);
        QVERIFY(circle.getFriends().empty());
        QCOMPARE(spy.count(), 1);
    }
}

void TestContactsCircle::removeFriendNotPresentedInCircle_FriendIsNotRemoved()
{
    ContactsCircle circle(0, "test");
    QSignalSpy spy(&circle, SIGNAL(friendRemoved(const ToxPk&)));
    const ToxPk friendToAddPk = PUBLIC_KEYS[0];
    const ToxPk friendToRemovePk = PUBLIC_KEYS[1];

    // remove from empty circle
    circle.removeFriend(friendToRemovePk);
    QVERIFY(circle.getFriends().empty());
    QVERIFY(spy.isEmpty());

    // remove from non-empty circle
    circle.addFriend(friendToAddPk);
    circle.removeFriend(friendToRemovePk);
    QCOMPARE(circle.getFriends().size(), 1ul);
    QVERIFY(spy.isEmpty());
}

void TestContactsCircle::removeFriend_isSorted()
{
    ContactsCircle circle(0, "test");
    for (const ToxPk pk: PUBLIC_KEYS) {
        circle.addFriend(pk);
    }

    for (const ToxPk pk: PUBLIC_KEYS) {
        circle.removeFriend(pk);
        QVERIFY(isSorted(circle.getFriends()));
    }
}

void TestContactsCircle::hasFriend_FriendIsFound()
{
    ContactsCircle circle(0, "test");
    const ToxPk friendPk = PUBLIC_KEYS[0];
    circle.addFriend(friendPk);
    QVERIFY(circle.hasFriend(friendPk));
}

void TestContactsCircle::hasFriend_FriendIsNotFound()
{
    ContactsCircle circle(0, "test");
    circle.addFriend(PUBLIC_KEYS[0]);
    QVERIFY(!circle.hasFriend(PUBLIC_KEYS[1]));
}

void TestContactsCircle::expansionChanged_SignalEmitted()
{
    ContactsCircle circle(0, "test");
    QSignalSpy spy(&circle, SIGNAL(expansionChanged(bool)));
    const bool initialValue = circle.isExpanded();
    circle.setExpanded(!initialValue);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> args = spy.takeFirst();
    const bool emittedValue = args.at(0).toBool();
    QVERIFY(initialValue != emittedValue);
}

void TestContactsCircle::expansionIsNotChanged_SignalIsNotEmitted()
{
    ContactsCircle circle(0, "test");
    QSignalSpy spy(&circle, SIGNAL(expansionChanged(bool)));
    const bool initialValue = circle.isExpanded();
    circle.setExpanded(initialValue);
    QVERIFY(spy.isEmpty());
}

void TestContactsCircle::circleRenamedSignalEmitted()
{
    const QString initialName = "test";
    ContactsCircle circle(0, initialName);
    QSignalSpy spy(&circle, SIGNAL(circleRenamed(QString)));
    const QString newName = "test1";
    QVERIFY(initialName != newName);
    circle.setCircleName(newName);
    QCOMPARE(spy.count(), 1);
    const QList<QVariant> args = spy.takeFirst();
    const QString emittedName = args.at(0).toString();
    QVERIFY(emittedName == newName);
}

void TestContactsCircle::invalidCircleName_CircleIsNotRenamed()
{
    const QString initialName = "test";
    ContactsCircle circle(0, initialName);
    QSignalSpy spy(&circle, SIGNAL(circleRenamed(QString)));
    const QString invalidNames[] = { initialName, "", "   " };
    for (const QString name: invalidNames) {
        circle.setCircleName(name);
        QVERIFY(circle.getCircleName() == initialName);
        QVERIFY(spy.isEmpty());
    }
}

QTEST_GUILESS_MAIN(TestContactsCircle)
#include "contactscircle_test.moc"
