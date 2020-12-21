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

#pragma once

#include "util/interface.h"

#include <QList>
#include <QNetworkProxy>
#include <QString>

class ICoreSettings {
public:
    enum class ProxyType
    {
        // If changed, don't forget to update Settings::fixInvalidProxyType
        ptNone = 0,
        ptSOCKS5 = 1,
        ptHTTP = 2
    };
    virtual ~ICoreSettings() = default;

    virtual bool getEnableIPv6() const = 0;
    virtual void setEnableIPv6(bool enable) = 0;

    virtual bool getForceTCP() const = 0;
    virtual void setForceTCP(bool enable) = 0;

    virtual bool getEnableLanDiscovery() const = 0;
    virtual void setEnableLanDiscovery(bool enable) = 0;

    virtual QString getProxyAddr() const = 0;
    virtual void setProxyAddr(const QString& address) = 0;

    virtual ProxyType getProxyType() const = 0;
    virtual void setProxyType(ProxyType type) = 0;

    virtual quint16 getProxyPort() const = 0;
    virtual void setProxyPort(quint16 port) = 0;

    virtual QNetworkProxy getProxy() const = 0;

    DECLARE_SIGNAL(enableIPv6Changed, bool enabled);
    DECLARE_SIGNAL(forceTCPChanged, bool enabled);
    DECLARE_SIGNAL(enableLanDiscoveryChanged, bool enabled);
    DECLARE_SIGNAL(proxyTypeChanged, ICoreSettings::ProxyType type);
    DECLARE_SIGNAL(proxyAddressChanged, const QString& address);
    DECLARE_SIGNAL(proxyPortChanged, quint16 port);
};
