#ifndef I_CORE_SETTINGS_H
#define I_CORE_SETTINGS_H

#include "src/model/interface.h"
#include "src/core/dhtserver.h"
#include "src/core/iproxysettings.h"

#include <QList>
#include <QString>

class ICoreSettings : public IProxySettings {
public:

    virtual bool getEnableIPv6() const = 0;
    virtual void setEnableIPv6(bool enable) = 0;

    virtual bool getForceTCP() const = 0;
    virtual void setForceTCP(bool enable) = 0;

    virtual bool getEnableLanDiscovery() const = 0;
    virtual void setEnableLanDiscovery(bool enable) = 0;

    virtual const QList<DhtServer>& getDhtServerList() const = 0;
    virtual void setDhtServerList(const QList<DhtServer>& servers) = 0;

    DECLARE_SIGNAL(enableIPv6Changed, bool enabled);
    DECLARE_SIGNAL(forceTCPChanged, bool enabled);
    DECLARE_SIGNAL(enableLanDiscoveryChanged, bool enabled);
    DECLARE_SIGNAL(dhtServerListChanged, const QList<DhtServer>& servers);
};

#endif // I_CORE_SETTINGS_H
