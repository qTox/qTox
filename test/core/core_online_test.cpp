/*
    Copyright © 2019 by The qTox Project Contributors

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

#include "src/core/core.h"
#include "src/core/toxoptions.h"
#include "src/core/icoresettings.h"
#include "src/net/bootstrapnodeupdater.h"
#include "src/model/ibootstraplistgenerator.h"
#include "src/persistence/settings.h"

#include <QtTest/QtTest>
#include <QtGlobal>
#include <limits>
#include <QSignalSpy>

#include <iostream>
#include <memory>

Q_DECLARE_METATYPE(QList<DhtServer>)

class MockSettings : public QObject, public ICoreSettings
{
Q_OBJECT
public:
    MockSettings() {
        Q_INIT_RESOURCE(res);
        qRegisterMetaType<QList<DhtServer>>("QList<DhtServer>");
    }

    bool getEnableIPv6() const override { return false; }
    void setEnableIPv6(bool) override { }

    bool getForceTCP() const override { return false; }
    void setForceTCP(bool) override { }

    bool getEnableLanDiscovery() const override { return false; }
    void setEnableLanDiscovery(bool) override { }

    QString getProxyAddr() const override { return Addr; }
    void setProxyAddr(const QString &Addr) override { this->Addr = Addr; }

    ProxyType getProxyType() const override { return type; }
    void setProxyType(ProxyType type) override { this->type = type; }

    quint16 getProxyPort() const override { return port; }
    void setProxyPort(quint16 port) override { this->port = port; }

    QNetworkProxy getProxy() const override { return QNetworkProxy(QNetworkProxy::ProxyType::NoProxy); }

    SIGNAL_IMPL(MockSettings, enableIPv6Changed, bool enabled)
    SIGNAL_IMPL(MockSettings, forceTCPChanged, bool enabled)
    SIGNAL_IMPL(MockSettings, enableLanDiscoveryChanged, bool enabled)
    SIGNAL_IMPL(MockSettings, proxyTypeChanged, ICoreSettings::ProxyType type)
    SIGNAL_IMPL(MockSettings, proxyAddressChanged, const QString& address)
    SIGNAL_IMPL(MockSettings, proxyPortChanged, quint16 port)

private:
    QList<DhtServer> dhtServerList;
    QString Addr;
    ProxyType type;
    quint16 port;
};

class MockNodeListGenerator : public IBootstrapListGenerator
{
    QList<DhtServer> getBootstrapnodes();
};

QList<DhtServer> MockNodeListGenerator::getBootstrapnodes() {
    return BootstrapNodeUpdater::loadDefaultBootstrapNodes();
}

namespace {
    const int timeout = 90000; //90 seconds timeout allowed for test
}

class TestCoreOnline : public QObject
{
Q_OBJECT
private slots:
    void init();
    void deinit();

    // actual test cases
    void change_name();

private:
    /* Test Variables */
    ToxCorePtr alice;
    ToxCorePtr bob;
};

void TestCoreOnline::init()
{
    auto settings = std::unique_ptr<MockSettings>(new MockSettings());

    MockNodeListGenerator nodesGenerator{};

    alice = Core::makeToxCore(QByteArray{}, nullptr, nodesGenerator, nullptr);
    QVERIFY2(alice != nullptr, "alice initialization failed");
    bob = Core::makeToxCore(QByteArray{}, nullptr, nodesGenerator, nullptr);
    QVERIFY2(bob != nullptr, "bob initialization failed");

    QSignalSpy spyAliceOnline(alice.get(), &Core::connected);
    QSignalSpy spyBobOnline(bob.get(), &Core::connected);

    alice->start();
    bob->start();

    QTest::qSleep(60000);

    // Wait for both instances coming online
    QTRY_VERIFY(spyAliceOnline.count() >= 1 && spyBobOnline.count() >= 1);

    // Make a friend request from alice to bob
    const QLatin1Literal friendMsg{"Test Invite Message"};

    QSignalSpy spyBobFriendMsg(bob.get(), &Core::friendRequestReceived);
    QSignalSpy spyAliceFriendMsg(alice.get(), &Core::requestSent);
    alice->requestFriendship(bob->getSelfId(), friendMsg);

    QTest::qSleep(30000);

    // Wait for both instances coming online
    QTRY_VERIFY(spyBobFriendMsg.count() == 1 && spyBobFriendMsg.count() == 1);

    // Check for expected signal content
    QVERIFY(spyBobFriendMsg[0][0].toByteArray() == alice->getSelfPublicKey().getByteArray());
    QVERIFY(spyBobFriendMsg[0][1].toString() == friendMsg);

    QVERIFY(spyAliceFriendMsg[0][0].toByteArray() == alice->getSelfPublicKey().getByteArray());
    QVERIFY(spyAliceFriendMsg[0][1].toString() == friendMsg);

    // Let Bob accept the friend request from Alice
    bob->acceptFriendRequest(alice->getSelfPublicKey());

    // Wait until both see each other
    QSignalSpy spyAliceFriendOnline(alice.get(), &Core::friendStatusChanged);
    QSignalSpy spyBobFriendOnline(bob.get(), &Core::friendStatusChanged);

    // FIXME: Check if this is reliable even with CoreExt
    // Wait for both instances being online
    QTRY_VERIFY_WITH_TIMEOUT(spyAliceFriendOnline.count() >= 1 && spyBobFriendOnline.count() >= 1, timeout);

    // Check for expected signal content
    QVERIFY(spyAliceFriendOnline[0][0].toInt() == static_cast<int>(Status::Status::Online));
    QVERIFY(spyBobFriendOnline[0][0].toInt() == static_cast<int>(Status::Status::Online));
}

void TestCoreOnline::deinit()
{
    alice.reset();
    bob.reset();
}

void TestCoreOnline::change_name()
{
    // Change the name of Alice to "Alice"
    const QLatin1Literal aliceName{"Alice"};

    QSignalSpy aliceSaveRequest(alice.get(), &Core::saveRequest);
    QSignalSpy aliceUsernameChanged(alice.get(), &Core::usernameSet);
    QSignalSpy bobUsernameChangeReceived(bob.get(), &Core::friendUsernameChanged);

    alice->setUsername(aliceName);

    QTRY_VERIFY_WITH_TIMEOUT(aliceSaveRequest.count() == 1, 1000);
    QTRY_VERIFY_WITH_TIMEOUT(aliceUsernameChanged.count() == 1 &&
                             aliceUsernameChanged[0][0].toString() == aliceName
                             , 1000);

    QTRY_VERIFY_WITH_TIMEOUT(bobUsernameChangeReceived.count() == 1 &&
                             bobUsernameChangeReceived[0][1].toString() == aliceName, timeout);
}

QTEST_GUILESS_MAIN(TestCoreOnline)
#include "core_online_test.moc"
