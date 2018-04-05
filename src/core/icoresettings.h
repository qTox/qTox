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

    virtual bool getEnableLanDiscovery() const = 0;
    virtual void setEnableLanDiscovery(bool enable) = 0;

    virtual QString getProxyAddr() const = 0;
    virtual void setProxyAddr(const QString& address) = 0;

    virtual ProxyType getProxyType() const = 0;
    virtual void setProxyType(ProxyType type) = 0;

    virtual quint16 getProxyPort() const = 0;
    virtual void setProxyPort(quint16 port) = 0;

    virtual const QList<DhtServer>& getDhtServerList() const = 0;
    virtual void setDhtServerList(const QList<DhtServer>& servers) = 0;

    virtual QNetworkProxy getProxy() const = 0;

    DECLARE_SIGNAL(enableIPv6Changed, bool enabled);
    DECLARE_SIGNAL(forceTCPChanged, bool enabled);
    DECLARE_SIGNAL(enableLanDiscoveryChanged, bool enabled);
    DECLARE_SIGNAL(proxyTypeChanged, ICoreSettings::ProxyType type);
    DECLARE_SIGNAL(proxyAddressChanged, const QString& address);
    DECLARE_SIGNAL(proxyPortChanged, quint16 port);
    DECLARE_SIGNAL(dhtServerListChanged, const QList<DhtServer>& servers);
};

#endif // I_CORE_SETTINGS_H
