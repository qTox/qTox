/*
    Copyright © 2018-2020 by The qTox Project Contributors

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

#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QObject>

#include "src/core/dhtserver.h"
#include "src/model/ibootstraplistgenerator.h"

class QNetworkReply;
class Paths;

class BootstrapNodeUpdater : public QObject, public IBootstrapListGenerator
{
    Q_OBJECT
public:
    explicit BootstrapNodeUpdater(const QNetworkProxy& proxy_, Paths& paths_, QObject* parent = nullptr);
    QList<DhtServer> getBootstrapNodes() const override;
    void requestBootstrapNodes();
    static QList<DhtServer> loadDefaultBootstrapNodes();

signals:
    void availableBootstrapNodes(QList<DhtServer> nodes);

private slots:
    void onRequestComplete(QNetworkReply* reply);

private:
    QList<DhtServer> loadUserBootrapNodes();

private:
    QNetworkProxy proxy;
    QNetworkAccessManager nam;
    Paths& paths;
};
