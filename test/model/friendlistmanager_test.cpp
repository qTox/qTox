/*
    Copyright © 2022 by The qTox Project Contributors

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

#include "src/model/friendlist/friendlistmanager.h"

#include <QTest>
#include <QSignalSpy>

class MockFriend : public IFriendListItem
{
public:
    MockFriend()
        : name("No Name"),
          lastActivity(QDateTime::currentDateTime()),
          online(false) {}

    MockFriend(const QString& nameStr, bool onlineRes, const QDateTime& lastAct)
        : name(nameStr),
          lastActivity(lastAct),
          online(onlineRes) {}

    ~MockFriend();

    bool isFriend() const override { return true; }
    bool isGroup() const override { return false; }
    bool isOnline() const override { return online; }
    bool widgetIsVisible() const override { return visible; }

    QString getNameItem() const override { return name; }
    QDateTime getLastActivity() const override { return lastActivity; }
    QWidget* getWidget() override { return nullptr; }

    void setWidgetVisible(bool v) override { visible = v; }

private:
    QString name;
    QDateTime lastActivity;
    bool online = false;
    bool visible = true;

};

MockFriend::~MockFriend() = default;

class MockGroup : public IFriendListItem
{
public:
    MockGroup()
        : name("group") {}

    MockGroup(const QString& nameStr)
        :name(nameStr) {}

    ~MockGroup();

    bool isFriend() const override { return false; }
    bool isGroup() const override { return true; }
    bool isOnline() const override { return true; }
    bool widgetIsVisible() const override { return visible; }

    QString getNameItem() const override { return name; }
    QDateTime getLastActivity() const override { return QDateTime::currentDateTime(); }
    QWidget* getWidget() override { return nullptr; }

    void setWidgetVisible(bool v) override { visible = v; }

private:
    QString name;
    bool visible = true;
};

MockGroup::~MockGroup() = default;

class FriendItemsBuilder
{
public:
    FriendItemsBuilder* addOfflineFriends()
    {
        QStringList testNames {".test", "123", "A test user", "Aatest user", "atest",
                              "btest", "ctest", "Test user", "user with long nickname one",
                              "user with long nickname two"};

        for (int i = 0; i < testNames.size(); ++i) {
            int unsortedIndex = i % 2 ? i - 1 : testNames.size() - i - 1; // Mixes positions
            int sortedByActivityIndex = testNames.size() - i - 1;
            unsortedAllFriends.append(testNames[unsortedIndex]);
            sortedByNameOfflineFriends.append(testNames[i]);
            sortedByActivityFriends.append(testNames[sortedByActivityIndex]);
        }

        return this;
    }

    FriendItemsBuilder* addOnlineFriends()
    {
        QStringList testNames {".test online", "123 online", "A test user online",
                              "Aatest user online", "atest online", "btest online", "ctest online",
                              "Test user online", "user with long nickname one online",
                              "user with long nickname two online"};

        for (int i = 0; i < testNames.size(); ++i) {
            int unsortedIndex = i % 2 ? i - 1 : testNames.size() - i - 1;
            int sortedByActivityIndex = testNames.size() - i - 1;
            unsortedAllFriends.append(testNames[unsortedIndex]);
            sortedByNameOnlineFriends.append(testNames[i]);
            sortedByActivityFriends.append(testNames[sortedByActivityIndex]);
        }

        return this;
    }

    FriendItemsBuilder* addGroups()
    {
        unsortedGroups.append("Test Group");
        unsortedGroups.append("A Group");
        unsortedGroups.append("Test Group long name");
        unsortedGroups.append("Test Group long aname");
        unsortedGroups.append("123");

        sortedByNameGroups.push_back("123");
        sortedByNameGroups.push_back("A Group");
        sortedByNameGroups.push_back("Test Group");
        sortedByNameGroups.push_back("Test Group long aname");
        sortedByNameGroups.push_back("Test Group long name");

        return this;
    }

    FriendItemsBuilder* setGroupsOnTop(bool val)
    {
        groupsOnTop = val;
        return this;
    }

    bool getGroupsOnTop()
    {
        return groupsOnTop;
    }

    /**
     * @brief buildUnsorted Creates items to init the FriendListManager.
     * FriendListManager will own and manage these items
     * @return Unsorted vector of items
     */
    QVector<IFriendListItem*> buildUnsorted()
    {
        checkDifferentNames();

        QVector<IFriendListItem*> vec;
        for (auto name : unsortedAllFriends) {
            vec.push_back(new MockFriend(name, isOnline(name), getDateTime(name)));
        }
        for (auto name : unsortedGroups) {
            vec.push_back(new MockGroup(name));
        }
        clear();
        return vec;
    }

    /**
     * @brief buildSortedByName Create items to compare with items
     * from the FriendListManager. FriendItemsBuilder owns these items
     * @return Sorted by name vector of items
     */
    QVector<std::shared_ptr<IFriendListItem>> buildSortedByName()
    {
        QVector<std::shared_ptr<IFriendListItem>> vec;
        if (!groupsOnTop) {
            for (auto name : sortedByNameOnlineFriends) {
                vec.push_back(std::shared_ptr<IFriendListItem>(new MockFriend(name, true, QDateTime::currentDateTime())));
            }

            for (auto name : sortedByNameGroups) {
                vec.push_back(std::shared_ptr<IFriendListItem>(new MockGroup(name)));
            }
        } else {
            for (auto name : sortedByNameGroups) {
                vec.push_back(std::shared_ptr<IFriendListItem>(new MockGroup(name)));
            }

            for (auto name : sortedByNameOnlineFriends) {
                vec.push_back(std::shared_ptr<IFriendListItem>(new MockFriend(name, true, QDateTime::currentDateTime())));
            }
        }

        for (auto name : sortedByNameOfflineFriends) {
            vec.push_back(std::shared_ptr<IFriendListItem>(new MockFriend(name, false, getDateTime(name))));
        }
        clear();
        return vec;
    }

    /**
     * @brief buildSortedByActivity Creates items to compare with items
     * from FriendListManager. FriendItemsBuilder owns these items
     * @return Sorted by activity vector of items
     */
    QVector<std::shared_ptr<IFriendListItem>> buildSortedByActivity()
    {
        QVector<std::shared_ptr<IFriendListItem>> vec;

        // Add groups on top
        for (auto name : sortedByNameGroups) {
            vec.push_back(std::shared_ptr<IFriendListItem>(new MockGroup(name)));
        }

        // Add friends and set the date of the last activity by index
        QDateTime dateTime = QDateTime::currentDateTime();
        for (int i = 0; i < sortedByActivityFriends.size(); ++i) {
            QString name = sortedByActivityFriends.at(i);
            vec.push_back(std::shared_ptr<IFriendListItem>(new MockFriend(name, isOnline(name), getDateTime(name))));
        }
        clear();
        return vec;
    }

private:
    void clear()
    {
        sortedByNameOfflineFriends.clear();
        sortedByNameOnlineFriends.clear();
        sortedByNameGroups.clear();
        sortedByActivityFriends.clear();
        sortedByActivityGroups.clear();
        unsortedAllFriends.clear();
        unsortedGroups.clear();
        groupsOnTop = true;
    }

    bool isOnline(const QString& name)
    {
        return sortedByNameOnlineFriends.indexOf(name) != -1;
    }

    /**
     * @brief checkDifferentNames The check is necessary for
     * the correct setting of the online status
     */
    void checkDifferentNames() {
        for (auto name : sortedByNameOnlineFriends) {
            if (sortedByNameOfflineFriends.contains(name, Qt::CaseInsensitive)) {
                QFAIL("Names in sortedByNameOnlineFriends and sortedByNameOfflineFriends "
                      "should be different");
                break;
            }
        }
    }

    QDateTime getDateTime(const QString& name)
    {
        QDateTime dateTime = QDateTime::currentDateTime();
        int pos = sortedByActivityFriends.indexOf(name);
        if (pos == -1) {
            return dateTime;
        }
        const int dayRatio = -1;
        return dateTime.addDays(dayRatio * pos * pos);
    }

    QStringList sortedByNameOfflineFriends;
    QStringList sortedByNameOnlineFriends;
    QStringList sortedByNameGroups;
    QStringList sortedByActivityFriends;
    QStringList sortedByActivityGroups;
    QStringList unsortedAllFriends;
    QStringList unsortedGroups;
    bool groupsOnTop = true;
};

class TestFriendListManager : public QObject
{
    Q_OBJECT
private slots:
    void testAddFriendListItem();
    void testSortByName();
    void testSortByActivity();
    void testSetFilter();
    void testApplyFilterSearchString();
    void testApplyFilterByStatus();
    void testSetGroupsOnTop();
private:
    std::unique_ptr<FriendListManager> createManagerWithItems(
            const QVector<IFriendListItem*> itemsVec);
};

void TestFriendListManager::testAddFriendListItem()
{
    auto manager = std::unique_ptr<FriendListManager>(new FriendListManager(0, this));
    QSignalSpy spy(manager.get(), &FriendListManager::itemsChanged);
    FriendItemsBuilder listBuilder;

    auto checkFunc = [&](const QVector<IFriendListItem*> itemsVec) {
        for (auto item : itemsVec) {
            manager->addFriendListItem(item);
        }
        QCOMPARE(manager->getItems().size(), itemsVec.size());
        QCOMPARE(spy.count(), itemsVec.size());
        spy.clear();
        for (auto item : itemsVec) {
            manager->removeFriendListItem(item);
        }
        QCOMPARE(manager->getItems().size(), 0);
        QCOMPARE(spy.count(), itemsVec.size());
        spy.clear();
    };

    // Only friends
    checkFunc(listBuilder.addOfflineFriends()->buildUnsorted());
    checkFunc(listBuilder.addOfflineFriends()->addOnlineFriends()->buildUnsorted());
    // Friends and groups
    checkFunc(listBuilder.addOfflineFriends()->addGroups()->buildUnsorted());
    checkFunc(listBuilder.addOfflineFriends()->addOnlineFriends()->addGroups()->buildUnsorted());
    // Only groups
    checkFunc(listBuilder.addGroups()->buildUnsorted());
}

void TestFriendListManager::testSortByName()
{
    FriendItemsBuilder listBuilder;
    auto unsortedVec = listBuilder.addOfflineFriends()
            ->addOnlineFriends()->addGroups()->buildUnsorted();
    auto sortedVec = listBuilder.addOfflineFriends()
            ->addOnlineFriends()->addGroups()->buildSortedByName();
    auto manager = createManagerWithItems(unsortedVec);

    manager->sortByName();
    bool success = manager->getPositionsChanged();
    manager->sortByName();

    QCOMPARE(success, true);
    QCOMPARE(manager->getPositionsChanged(), false);
    QCOMPARE(manager->getItems().size(), sortedVec.size());
    QCOMPARE(manager->getGroupsOnTop(),  listBuilder.getGroupsOnTop());

    for (int i = 0; i < sortedVec.size(); ++i) {
        IFriendListItem* fromManager = manager->getItems().at(i).get();
        std::shared_ptr<IFriendListItem> fromSortedVec = sortedVec.at(i);
        QCOMPARE(fromManager->getNameItem(), fromSortedVec->getNameItem());
    }
}

void TestFriendListManager::testSortByActivity()
{
    FriendItemsBuilder listBuilder;
    auto unsortedVec = listBuilder.addOfflineFriends()
            ->addOnlineFriends()->addGroups()->buildUnsorted();
    auto sortedVec = listBuilder.addOfflineFriends()
            ->addOnlineFriends()->addGroups()->buildSortedByActivity();

    std::unique_ptr<FriendListManager> manager = createManagerWithItems(unsortedVec);
    manager->sortByActivity();
    bool success = manager->getPositionsChanged();
    manager->sortByActivity();

    QCOMPARE(success, true);
    QCOMPARE(manager->getPositionsChanged(), false);
    QCOMPARE(manager->getItems().size(), sortedVec.size());
    for (int i = 0; i < sortedVec.size(); ++i) {
        auto fromManager = manager->getItems().at(i).get();
        auto fromSortedVec = sortedVec.at(i);
        QCOMPARE(fromManager->getNameItem(), fromSortedVec->getNameItem());
    }
}

void TestFriendListManager::testSetFilter()
{
    FriendItemsBuilder listBuilder;
    auto manager = createManagerWithItems(
                listBuilder.addOfflineFriends()->addOnlineFriends()->addGroups()->buildUnsorted());
    QSignalSpy spy(manager.get(), &FriendListManager::itemsChanged);

    manager->setFilter("", false, false, false);

    QCOMPARE(spy.count(), 0);

    manager->setFilter("Test", true, false, false);
    manager->setFilter("Test", true, false, false);

    QCOMPARE(spy.count(), 1);
}

void TestFriendListManager::testApplyFilterSearchString()
{
    FriendItemsBuilder listBuilder;
    auto manager = createManagerWithItems(
                listBuilder.addOfflineFriends()->addOnlineFriends()->addGroups()->buildUnsorted());
    QVector<std::shared_ptr<IFriendListItem>> resultVec;
    QString testNameA = "NOITEMSWITHTHISNAME";
    QString testNameB = "Test Name B";
    manager->sortByName();
    manager->setFilter(testNameA, false, false, false);
    manager->applyFilter();

    resultVec = manager->getItems();
    for (auto item : resultVec) {
        QCOMPARE(item->widgetIsVisible(), false);
    }

    manager->sortByActivity();
    manager->addFriendListItem(new MockFriend(testNameB, true, QDateTime::currentDateTime()));
    manager->applyFilter();

    resultVec = manager->getItems();
    for (auto item : resultVec) {
        QCOMPARE(item->widgetIsVisible(), false);
    }

    manager->addFriendListItem(new MockFriend(testNameA, true, QDateTime::currentDateTime()));
    manager->applyFilter();

    resultVec = manager->getItems();
    for (auto item : resultVec) {
        if (item->getNameItem() == testNameA) {
            QCOMPARE(item->widgetIsVisible(), true);
        } else {
            QCOMPARE(item->widgetIsVisible(), false);
        }
    }

    manager->setFilter("", false, false, false);
    manager->applyFilter();

    resultVec = manager->getItems();
    for (auto item : resultVec) {
        QCOMPARE(item->widgetIsVisible(), true);
    }
}

void TestFriendListManager::testApplyFilterByStatus()
{
    FriendItemsBuilder listBuilder;
    auto manager = createManagerWithItems(
                listBuilder.addOfflineFriends()->addOnlineFriends()->addGroups()->buildUnsorted());
    auto onlineItems = listBuilder.addOnlineFriends()->buildSortedByName();
    auto offlineItems = listBuilder.addOfflineFriends()->buildSortedByName();
    auto groupItems = listBuilder.addGroups()->buildSortedByName();
    manager->sortByName();

    manager->setFilter("", true /*hideOnline*/, false /*hideOffline*/, false /*hideGroups*/);
    manager->applyFilter();

    for (auto item : manager->getItems()) {
        if (item->isOnline() && item->isFriend()) {
            QCOMPARE(item->widgetIsVisible(), false);
        } else {
            QCOMPARE(item->widgetIsVisible(), true);
        }
    }

    manager->setFilter("", false /*hideOnline*/, true /*hideOffline*/, false /*hideGroups*/);
    manager->applyFilter();

    for (auto item : manager->getItems()) {
        if (item->isOnline()) {
            QCOMPARE(item->widgetIsVisible(), true);
        } else {
            QCOMPARE(item->widgetIsVisible(), false);
        }
    }

    manager->setFilter("", false /*hideOnline*/, false /*hideOffline*/, true /*hideGroups*/);
    manager->applyFilter();

    for (auto item : manager->getItems()) {
        if (item->isGroup()) {
            QCOMPARE(item->widgetIsVisible(), false);
        } else {
            QCOMPARE(item->widgetIsVisible(), true);
        }
    }

    manager->setFilter("", true /*hideOnline*/, true /*hideOffline*/, true /*hideGroups*/);
    manager->applyFilter();

    for (auto item : manager->getItems()) {
        QCOMPARE(item->widgetIsVisible(), false);
    }

    manager->setFilter("", false /*hideOnline*/, false /*hideOffline*/, false /*hideGroups*/);
    manager->applyFilter();

    for (auto item : manager->getItems()) {
        QCOMPARE(item->widgetIsVisible(), true);
    }
}

void TestFriendListManager::testSetGroupsOnTop()
{
    FriendItemsBuilder listBuilder;
    auto manager = createManagerWithItems(
                listBuilder.addOfflineFriends()->addOnlineFriends()->addGroups()->buildUnsorted());
    auto sortedVecOnlineOnTop = listBuilder.addOfflineFriends()->addOnlineFriends()->addGroups()
            ->setGroupsOnTop(false)->buildSortedByName();
    auto sortedVecGroupsOnTop = listBuilder.addOfflineFriends()->addOnlineFriends()->addGroups()
            ->setGroupsOnTop(true)->buildSortedByName();

    manager->setGroupsOnTop(false);
    manager->sortByName();

    for (int i = 0; i < manager->getItems().size(); ++i) {
        auto fromManager = manager->getItems().at(i);
        auto fromSortedVec = sortedVecOnlineOnTop.at(i);
        QCOMPARE(fromManager->getNameItem(), fromSortedVec->getNameItem());
    }

    manager->setGroupsOnTop(true);
    manager->sortByName();

    for (int i = 0; i < manager->getItems().size(); ++i) {
        auto fromManager = manager->getItems().at(i);
        auto fromSortedVec = sortedVecGroupsOnTop.at(i);
        QCOMPARE(fromManager->getNameItem(), fromSortedVec->getNameItem());
    }
}

std::unique_ptr<FriendListManager> TestFriendListManager::createManagerWithItems(
        const QVector<IFriendListItem*> itemsVec)
{
    std::unique_ptr<FriendListManager> manager =
         std::unique_ptr<FriendListManager>(new FriendListManager(0, this));

    for (auto item : itemsVec) {
        manager->addFriendListItem(item);
    }

    return manager;
}

QTEST_GUILESS_MAIN(TestFriendListManager)
#include "friendlistmanager_test.moc"
