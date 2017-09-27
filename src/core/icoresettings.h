#ifndef I_CORE_SETTINGS_H
#define I_CORE_SETTINGS_H

#include "src/model/interface.h"
#include "src/core/dhtserver.h"

#include <QList>
#include <QNetworkProxy>
#include <QString>

class ICoreSettings {
public:
    enum class ProxyType
    {
        ptNone = 0,
        ptSOCKS5 = 1,
        ptHTTP = 2
    };

    virtual bool getEnableIPv6() const = 0;
    virtual void setEnableIPv6(bool enable) = 0;

    virtual bool getForceTCP() const = 0;
    virtual void setForceTCP(bool enable) = 0;

    virtual QString getProxyAddr() const = 0;
    virtual void setProxyAddr(const QString& address) = 0;

    virtual ProxyType getProxyType() const = 0;
    virtual void setProxyType(ProxyType type) = 0;

    virtual quint16 getProxyPort() const = 0;
    virtual void setProxyPort(quint16 port) = 0;

    virtual const QList<DhtServer>& getDhtServerList() const = 0;
    virtual void setDhtServerList(const QList<DhtServer>& servers) = 0;

    virtual QNetworkProxy getProxy() const = 0;

    CHANGED_SIGNAL(enableIPv6, bool enabled);
    CHANGED_SIGNAL(forceTCP, bool enabled);
    CHANGED_SIGNAL(proxyType, ICoreSettings::ProxyType type);
    CHANGED_SIGNAL(proxyAddress, const QString& address);
    CHANGED_SIGNAL(proxyPort, quint16 port);
    CHANGED_SIGNAL(dhtServerList, const QList<DhtServer>& servers);
};

#endif // I_CORE_SETTINGS_H
