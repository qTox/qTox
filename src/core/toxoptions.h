#ifndef TOXOPTIONS_H
#define TOXOPTIONS_H

#include <QByteArray>

#include <memory>

class ICoreSettings;
struct Tox_Options;

class ToxOptions
{
public:
    ~ToxOptions();
    ToxOptions(ToxOptions&& from);
    operator Tox_Options*();
    const char* getProxyAddrData() const;
    static std::unique_ptr<ToxOptions> makeToxOptions(const QByteArray& savedata,
                                                      const ICoreSettings* s);
    bool getIPv6Enabled() const;
    void setIPv6Enabled(bool enabled);

private:
    ToxOptions(Tox_Options* options, const QByteArray& proxyAddrData);

private:
    Tox_Options* options = nullptr;
    QByteArray proxyAddrData;
};

#endif // TOXOPTIONS_H
