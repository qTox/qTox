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

#pragma once

#include "src/core/dhtserver.h"
#include "src/core/icoresettings.h"
#include "src/core/toxpk.h"
#include "src/model/status.h"

#include <QList>
#include <QObject>

class MockSettings : public QObject, public ICoreSettings
{
    Q_OBJECT
public:
    MockSettings()
    {
        Q_INIT_RESOURCE(res);
    }

    ~MockSettings();

    bool getEnableIPv6() const override
    {
        return false;
    }
    void setEnableIPv6(bool) override {}

    bool getForceTCP() const override
    {
        return false;
    }
    void setForceTCP(bool) override {}

    bool getEnableLanDiscovery() const override
    {
        return false;
    }
    void setEnableLanDiscovery(bool) override {}

    QString getProxyAddr() const override
    {
        return Addr;
    }
    void setProxyAddr(const QString& Addr) override
    {
        this->Addr = Addr;
    }

    ProxyType getProxyType() const override
    {
        return type;
    }
    void setProxyType(ProxyType type) override
    {
        this->type = type;
    }

    quint16 getProxyPort() const override
    {
        return port;
    }
    void setProxyPort(quint16 port) override
    {
        this->port = port;
    }

    QNetworkProxy getProxy() const override
    {
        return QNetworkProxy(QNetworkProxy::ProxyType::NoProxy);
    }

    SIGNAL_IMPL(MockSettings, enableIPv6Changed, bool enabled)
    SIGNAL_IMPL(MockSettings, forceTCPChanged, bool enabled)
    SIGNAL_IMPL(MockSettings, enableLanDiscoveryChanged, bool enabled)
    SIGNAL_IMPL(MockSettings, proxyTypeChanged, ICoreSettings::ProxyType type)
    SIGNAL_IMPL(MockSettings, proxyAddressChanged, const QString& address)
    SIGNAL_IMPL(MockSettings, proxyPortChanged, quint16 port)

private:
    QString Addr;
    ProxyType type;
    quint16 port;
};
