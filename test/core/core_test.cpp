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

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <src/persistence/settings.h>
#include <iostream>

Q_DECLARE_METATYPE(QList<DhtServer>);

class MockSettings : public QObject, public ICoreSettings
{
Q_OBJECT
public:
    MockSettings() {
        qRegisterMetaType<QList<DhtServer>>("QList<DhtServer>");
    }

    bool getEnableIPv6() const override { return false; }
    void setEnableIPv6(bool) override { }

    bool getForceTCP() const override { return false; }
    void setForceTCP(bool) override { }

    bool getEnableLanDiscovery() const override { return false; }
    void setEnableLanDiscovery(bool) override { }

    QString getProxyAddr() const override { return QString(""); }
    void setProxyAddr(const QString &) override { }

    ProxyType getProxyType() const override { return ProxyType::ptNone; }
    void setProxyType(ProxyType) override { }

    quint16 getProxyPort() const override { return 0; }
    void setProxyPort(quint16) override { }

    QNetworkProxy getProxy() const override { return QNetworkProxy(QNetworkProxy::ProxyType::NoProxy); }

    SIGNAL_IMPL(MockSettings, enableIPv6Changed, bool enabled)
    SIGNAL_IMPL(MockSettings, forceTCPChanged, bool enabled)
    SIGNAL_IMPL(MockSettings, enableLanDiscoveryChanged, bool enabled)
    SIGNAL_IMPL(MockSettings, proxyTypeChanged, ICoreSettings::ProxyType type)
    SIGNAL_IMPL(MockSettings, proxyAddressChanged, const QString& address)
    SIGNAL_IMPL(MockSettings, proxyPortChanged, quint16 port)

private:
    QList<DhtServer> dhtServerList;

};


class TestCore : public QObject
{
Q_OBJECT
private slots:
    void startup_without_proxy();

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
    Q_INIT_RESOURCE(res);

    test_core = Core::makeToxCore(savedata, settings, err);

    if(test_core == ToxCorePtr{}) {
        QFAIL("ToxCore initialisation failed");
    }


    QSignalSpy spyCore(test_core.get(), &Core::connected);

    test_core->start();

    QVERIFY(spyCore.wait(timeout)); //wait 90seconds

    QCOMPARE(spyCore.count(), 1); // make sure the signal was emitted exactly one time
}

QTEST_GUILESS_MAIN(TestCore)
#include "core_test.moc"
