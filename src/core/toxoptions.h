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
