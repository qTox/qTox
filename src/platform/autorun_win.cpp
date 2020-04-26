/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include <QApplication>
#include "src/persistence/settings.h"
#include "src/platform/autorun.h"
#include <string>
#include <windows.h>

#ifdef UNICODE
/**
 * tstring is either std::wstring or std::string, depending on whether the user
 * is building a Unicode or Multi-Byte version of qTox. This makes the code
 * easier to reuse and compatible with both setups.
 */
using tstring = std::wstring;
static inline tstring toTString(QString s)
{
    return s.toStdWString();
}
#else
using tstring = std::string;
static inline tstring toTString(QString s)
{
    return s.toStdString();
}
#endif

namespace Platform {
inline tstring currentCommandLine()
{
    return toTString("\"" + QApplication::applicationFilePath().replace('/', '\\') + "\" -p \""
                     + Settings::getInstance().getCurrentProfile() + "\"");
}

inline tstring currentRegistryKeyName()
{
    return toTString("qTox - " + Settings::getInstance().getCurrentProfile());
}
}

bool Platform::setAutorun(bool on)
{
    HKEY key = nullptr;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                     0, KEY_ALL_ACCESS, &key)
        != ERROR_SUCCESS)
        return false;

    bool result = false;
    tstring keyName = currentRegistryKeyName();

    if (on) {
        tstring path = currentCommandLine();
        result = RegSetValueEx(key, keyName.c_str(), 0, REG_SZ, const_cast<PBYTE>(reinterpret_cast<const unsigned char*>(path.c_str())),
                               path.length() * sizeof(TCHAR))
                 == ERROR_SUCCESS;
    } else
        result = RegDeleteValue(key, keyName.c_str()) == ERROR_SUCCESS;

    RegCloseKey(key);
    return result;
}

bool Platform::getAutorun()
{
    HKEY key = nullptr;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                     0, KEY_ALL_ACCESS, &key)
        != ERROR_SUCCESS)
        return false;

    tstring keyName = currentRegistryKeyName();

    TCHAR path[MAX_PATH] = {0};
    DWORD length = sizeof(path);
    DWORD type = REG_SZ;
    bool result = false;

    if (RegQueryValueEx(key, keyName.c_str(), nullptr, &type, const_cast<PBYTE>(reinterpret_cast<const unsigned char*>(path)), &length) == ERROR_SUCCESS
        && type == REG_SZ)
        result = true;

    RegCloseKey(key);
    return result;
}
