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

#include "mock/mockbootstraplistgenerator.h"
#include "src/core/core.h"
#include "src/core/toxoptions.h"
#include "src/core/icoresettings.h"
#include "src/net/bootstrapnodeupdater.h"
#include "src/model/ibootstraplistgenerator.h"
#include "src/persistence/settings.h"
#include "mock/mockcoresettings.h"

#include <QtTest/QtTest>
#include <QtGlobal>
#include <limits>
#include <QSignalSpy>

#include <iostream>
#include <memory>

Q_DECLARE_METATYPE(QList<DhtServer>)

class TestCoreProxy : public QObject
{
Q_OBJECT
private slots:
    void startup_without_proxy();
    void startup_with_invalid_socks5_proxy();
    void startup_with_invalid_http_proxy();

private:
    /* Test Variables */
    Core::ToxCoreErrors* err = nullptr;
    MockSettings settings;
    ToxCorePtr alice;
    ToxCorePtr bob;
    MockBootstrapListGenerator alicesNodes{};
    MockBootstrapListGenerator bobsNodes{};
};


namespace {
const int timeout = 15000;

void bootstrapToxes(Core& alice, MockBootstrapListGenerator& alicesNodes,
    Core& bob, MockBootstrapListGenerator& bobsNodes)
{
    alicesNodes.setBootstrapNodes(MockBootstrapListGenerator::makeListFromSelf(bob));
    bobsNodes.setBootstrapNodes(MockBootstrapListGenerator::makeListFromSelf(alice));

    QSignalSpy spyAlice(&alice, &Core::connected);
    QSignalSpy spyBob(&bob, &Core::connected);

    alice.start();
    bob.start();

    QTRY_VERIFY_WITH_TIMEOUT(spyAlice.count() == 1 &&
        spyBob.count() == 1, timeout);
}
} // namespace

void TestCoreProxy::startup_without_proxy()
{
    // No proxy
    settings.setProxyAddr("");
    settings.setProxyPort(0);
    settings.setProxyType(MockSettings::ProxyType::ptNone);

    alice = Core::makeToxCore({}, settings, alicesNodes, err);
    bob = Core::makeToxCore({}, settings, bobsNodes, err);
    QVERIFY(!!alice);
    QVERIFY(!!bob);

    bootstrapToxes(*alice, alicesNodes, *bob, bobsNodes);
}

void TestCoreProxy::startup_with_invalid_socks5_proxy()
{
    settings.setProxyAddr("Test");
    settings.setProxyPort(9985);
    settings.setProxyType(MockSettings::ProxyType::ptSOCKS5);

    alice = Core::makeToxCore({}, settings, alicesNodes, err);

    QVERIFY(!alice);
}

void TestCoreProxy::startup_with_invalid_http_proxy()
{
    settings.setProxyAddr("Test");
    settings.setProxyPort(9985);
    settings.setProxyType(MockSettings::ProxyType::ptHTTP);

    alice = Core::makeToxCore({}, settings, alicesNodes, err);

    QVERIFY(!alice);
}

QTEST_GUILESS_MAIN(TestCoreProxy)
#include "core_test.moc"
