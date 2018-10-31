#include "toxoptions.h"

#include "src/core/icoresettings.h"
#include "src/core/toxlogger.h"

#include <tox/tox.h>

#include <QByteArray>
#include <QDebug>

/**
 * @brief The ToxOptions class wraps the Tox_Options struct and the matching
 *        proxy address data. This is needed to ensure both have equal lifetime and
 *        are correctly deleted.
 */

ToxOptions::ToxOptions(Tox_Options* options, const QByteArray& proxyAddrData)
    : options(options)
    , proxyAddrData(proxyAddrData)
{}

ToxOptions::~ToxOptions()
{
    tox_options_free(options);
}

ToxOptions::ToxOptions(ToxOptions&& from)
{
    options = from.options;
    proxyAddrData.swap(from.proxyAddrData);
    from.options = nullptr;
    from.proxyAddrData.clear();
}

const char* ToxOptions::getProxyAddrData() const
{
    return proxyAddrData.constData();
}

ToxOptions::operator Tox_Options*()
{
    return options;
}

/**
 * @brief Initializes a ToxOptions instance
 * @param savedata Previously saved Tox data
 * @return ToxOptions instance initialized to create Tox instance
 */
std::unique_ptr<ToxOptions> ToxOptions::makeToxOptions(const QByteArray& savedata,
                                                       const ICoreSettings* s)
{
    Tox_Options* tox_opts = tox_options_new(nullptr);

    if (!tox_opts) {
        qWarning() << "failed to create Tox_Options";
        return {};
    }

    // need to init proxyAddr here, because we need it to construct ToxOptions
    const QString proxyAddr = s == nullptr ? QString{} : s->getProxyAddr();

    auto toxOptions = std::unique_ptr<ToxOptions>(new ToxOptions(tox_opts, proxyAddr.toUtf8()));
    // register log first, to get messages as early as possible
    tox_options_set_log_callback(*toxOptions, ToxLogger::onLogMessage);

    // savedata
    tox_options_set_savedata_type(*toxOptions, savedata.isNull() ? TOX_SAVEDATA_TYPE_NONE
                                                                 : TOX_SAVEDATA_TYPE_TOX_SAVE);
    tox_options_set_savedata_data(*toxOptions, reinterpret_cast<const uint8_t*>(savedata.data()),
                                  savedata.size());

    if(s == nullptr) {
        qDebug() << "Using Tox default settings";
        return toxOptions;
    }

    // IPv6 needed for LAN discovery, but can crash some weird routers. On by default, can be
    // disabled in options.
    const bool enableIPv6 = s->getEnableIPv6();
    bool forceTCP = s->getForceTCP();
    // LAN requiring UDP is a toxcore limitation, ideally wouldn't be related
    const bool enableLanDiscovery = s->getEnableLanDiscovery() && !forceTCP;
    ICoreSettings::ProxyType proxyType = s->getProxyType();
    quint16 proxyPort = s->getProxyPort();

    if (!enableLanDiscovery) {
        qWarning() << "Core starting without LAN discovery. Peers can only be found through DHT.";
    }
    if (enableIPv6) {
        qDebug() << "Core starting with IPv6 enabled";
    } else if (enableLanDiscovery) {
        qWarning() << "Core starting with IPv6 disabled. LAN discovery may not work properly.";
    }

    // No proxy by default
    tox_options_set_proxy_type(*toxOptions, TOX_PROXY_TYPE_NONE);
    tox_options_set_proxy_host(*toxOptions, nullptr);
    tox_options_set_proxy_port(*toxOptions, 0);

    if (proxyType != ICoreSettings::ProxyType::ptNone) {
        if (static_cast<uint32_t>(proxyAddr.length()) > tox_max_hostname_length()) {
            qWarning() << "proxy address" << proxyAddr << "is too long";
        } else if (!proxyAddr.isEmpty() && proxyPort > 0) {
            qDebug() << "using proxy" << proxyAddr << ":" << proxyPort;
            // protection against changings in Tox_Proxy_Type enum
            if (proxyType == ICoreSettings::ProxyType::ptSOCKS5) {
                tox_options_set_proxy_type(*toxOptions, TOX_PROXY_TYPE_SOCKS5);
            } else if (proxyType == ICoreSettings::ProxyType::ptHTTP) {
                tox_options_set_proxy_type(*toxOptions, TOX_PROXY_TYPE_HTTP);
            }

            tox_options_set_proxy_host(*toxOptions, toxOptions->getProxyAddrData());
            tox_options_set_proxy_port(*toxOptions, proxyPort);

            if (!forceTCP) {
                qDebug() << "Proxy and UDP enabled, this is a security risk, forcing TCP only";
                forceTCP = true;
            }
        }
    }

    // network options
    tox_options_set_udp_enabled(*toxOptions, !forceTCP);
    tox_options_set_ipv6_enabled(*toxOptions, enableIPv6);
    tox_options_set_local_discovery_enabled(*toxOptions, enableLanDiscovery);
    tox_options_set_start_port(*toxOptions, 0);
    tox_options_set_end_port(*toxOptions, 0);

    return toxOptions;
}

bool ToxOptions::getIPv6Enabled() const
{
    return tox_options_get_ipv6_enabled(options);
}

void ToxOptions::setIPv6Enabled(bool enabled)
{
    tox_options_set_ipv6_enabled(options, enabled);
}
