/*
    Copyright © 2018-2019 by The qTox Project Contributors

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

#include "src/net/bootstrapnodeupdater.h"
#include "src/persistence/paths.h"

#include <QNetworkProxy>
#include <QSignalSpy>

#include <QtTest/QtTest>

// Needed to make this type known to Qt
Q_DECLARE_METATYPE(QList<DhtServer>)

class TestBootstrapNodesUpdater : public QObject
{
    Q_OBJECT
public:
    TestBootstrapNodesUpdater();
private slots:
    void testOnline();
    void testLocal();
};

TestBootstrapNodesUpdater::TestBootstrapNodesUpdater()
{
    qRegisterMetaType<QList<DhtServer>>("QList<DhtServer>");
    // Contains the builtin nodes list
    Q_INIT_RESOURCE(res);
}

void TestBootstrapNodesUpdater::testOnline()
{
    QNetworkProxy proxy{QNetworkProxy::ProxyType::NoProxy};
    auto paths = Paths::makePaths(Paths::Portable::NonPortable);

    BootstrapNodeUpdater updater{proxy, *paths};
    QSignalSpy spy(&updater, &BootstrapNodeUpdater::availableBootstrapNodes);

    updater.requestBootstrapNodes();

    spy.wait(10000); // increase wait time for speradic CI failures with slow nodes server
    QCOMPARE(spy.count(), 1); // make sure the signal was emitted exactly one time
    QList<DhtServer> result = qvariant_cast<QList<DhtServer>>(spy.at(0).at(0));
    QVERIFY(result.size() > 0); // some data should be returned
}

void TestBootstrapNodesUpdater::testLocal()
{
    QList<DhtServer> defaultNodes = BootstrapNodeUpdater::loadDefaultBootstrapNodes();
    QVERIFY(defaultNodes.size() > 0);
}

QTEST_GUILESS_MAIN(TestBootstrapNodesUpdater)
#include "bsu_test.moc"
