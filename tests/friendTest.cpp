#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mocks/mockCore.h"
#include "mocks/mockProfile.h"
#include "mocks/mockSettings.h"

#include "src/friend.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"

using ::testing::Return;
using ::testing::_;

class FriendTest : public ::testing::Test
{
protected:
    FriendTest()
    {
        toxId = ToxId(testId);
        EXPECT_CALL(*core, getPeerName(toxId))
                .WillRepeatedly(Return(QString(friendName)));
        EXPECT_CALL(*s, getFriendAlias(toxId))
                .WillRepeatedly(Return(QString()));
    }

    static void SetUpTestCase()
    {
        s = new MockSettings;
        EXPECT_CALL(*s, loadPersonal(_)).Times(1);

        profile = new MockProfile(QStringLiteral("test"), QString(), false);
        coreThread = new QThread();
        coreThread->setObjectName("qTox Core");
        core = new MockCore(coreThread, *profile);
        profile->setCore(core);
        Nexus::setProfile(profile);
    }

    static void TearDownTestCase()
    {
        delete profile;
        delete s;
    }

    virtual ~FriendTest()
    {
    }

    static MockSettings *s;
    static MockProfile *profile;

    static QThread *coreThread;
    static MockCore *core;

    const uint32_t friendId = 1;
    const QString testId = "0000000000000000000000000000000000000000000000000000000000000000000000000000";
    const QString friendName = "FriendName";
    ToxId toxId;
};

MockSettings *FriendTest::s;
MockProfile *FriendTest::profile;
QThread *FriendTest::coreThread;
MockCore *FriendTest::core;

TEST_F(FriendTest, init)
{
    Friend f(friendId, toxId);
    ASSERT_EQ(toxId, f.getToxId());
    ASSERT_EQ(friendId, f.getFriendID());
}

TEST_F(FriendTest, name)
{
    EXPECT_CALL(*s, getFriendAlias(toxId))
            .WillRepeatedly(Return(QString()));

    Friend f(friendId, toxId);

    ASSERT_FALSE(f.hasAlias());

    QString displayedName = f.getDisplayedName();
    ASSERT_EQ(friendName, displayedName) << displayedName.toStdString();

    const QString newName = "NewFriendName";
    f.setName(newName);
    displayedName = f.getDisplayedName();
    ASSERT_EQ(newName, displayedName) << displayedName.toStdString();

    const QString alias = "Alias";
    f.setAlias(alias);
    ASSERT_TRUE(f.hasAlias());
    displayedName = f.getDisplayedName();
    ASSERT_EQ(alias, displayedName) << displayedName.toStdString();
}

TEST_F(FriendTest, statusMessage)
{
    Friend f(friendId, toxId);
    const QString status = "Test status";
    f.setStatusMessage(status);
    ASSERT_EQ(status, f.getStatusMessage());
}

TEST_F(FriendTest, status)
{
    Friend f(friendId, toxId);
    Status s = Status::Online;
    f.setStatus(s);
    ASSERT_EQ(s, f.getStatus());
}
