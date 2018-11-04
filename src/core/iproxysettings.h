#ifndef IPROXYSETTINGS_H
#define IPROXYSETTINGS_H

#include "src/model/interface.h"
#include <QNetworkProxy>
#include <QString>

class IProxySettings {
public:
    enum class ProxyType
    {
        None = 0,
        SOCKS5 = 1,
        HTTP = 2
    };

    virtual QString getProxyAddr() const = 0;
    virtual void setProxyAddr(const QString& address) = 0;

    virtual ProxyType getProxyType() const = 0;
    virtual void setProxyType(ProxyType type) = 0;

    virtual quint16 getProxyPort() const = 0;
    virtual void setProxyPort(quint16 port) = 0;

    virtual QNetworkProxy getProxy() const = 0;

    DECLARE_SIGNAL(proxyTypeChanged, IProxySettings::ProxyType type);
    DECLARE_SIGNAL(proxyAddressChanged, const QString& address);
    DECLARE_SIGNAL(proxyPortChanged, quint16 port);

    virtual ~IProxySettings(){}
};
#endif // IPROXYSETTINGS_H
