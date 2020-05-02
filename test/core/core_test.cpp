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

#include <QtTest/QtTest>
#include <QtGlobal>
#include <limits>
#include <QSignalSpy>
#include <src/persistence/settings.h>
#include <iostream>

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
    QList<DhtServer> getBootstrapnodes() {
        return BootstrapNodeUpdater::loadDefaultBootstrapNodes();
    }
};

class TestCore : public QObject
{
Q_OBJECT
private slots:
    void startup_without_proxy();
    void startup_with_invalid_proxy();

private:
    /* Test Variables */
    Core::ToxCoreErrors* err = nullptr;
    MockSettings* settings;
    QByteArray savedata{};
    ToxCorePtr test_core;
};


namespace {
    const int timeout = 90000; //90 seconds timeout allowed for test
}

void TestCore::startup_without_proxy()
{
    settings = new MockSettings();

    // No proxy
    settings->setProxyAddr("");
    settings->setProxyPort(0);
    settings->setProxyType(MockSettings::ProxyType::ptNone);

    MockNodeListGenerator nodesGenerator{};

    test_core = Core::makeToxCore(savedata, settings, nodesGenerator, err);

    if (test_core == nullptr) {
        QFAIL("ToxCore initialisation failed");
    }


    QSignalSpy spyCore(test_core.get(), &Core::connected);

    test_core->start();

    QVERIFY(spyCore.wait(timeout)); //wait 90seconds

    QCOMPARE(spyCore.count(), 1); // make sure the signal was emitted exactly one time
}

void TestCore::startup_with_invalid_proxy()
{
    settings = new MockSettings();


    // Test invalid proxy SOCKS5
    settings->setProxyAddr("Test");
    settings->setProxyPort(9985);
    settings->setProxyType(MockSettings::ProxyType::ptSOCKS5);

    MockNodeListGenerator nodesGenerator{};

    test_core = Core::makeToxCore(savedata, settings, nodesGenerator, err);

    if (test_core != nullptr) {
        QFAIL("ToxCore initialisation passed with invalid SOCKS5 proxy address");
    }


    // Test invalid proxy HTTP
    settings->setProxyAddr("Test");
    settings->setProxyPort(9985);
    settings->setProxyType(MockSettings::ProxyType::ptHTTP);

    test_core = Core::makeToxCore(savedata, settings, nodesGenerator, err);

    if (test_core != nullptr) {
        QFAIL("ToxCore initialisation passed with invalid HTTP proxy address");
    }
}

QTEST_GUILESS_MAIN(TestCore)
#include "core_test.moc"
