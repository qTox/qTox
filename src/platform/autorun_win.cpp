/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#include <QApplication>
#ifdef Q_OS_WIN32
#include "src/platform/autorun.h"
#include "src/misc/settings.h"
#include <windows.h>
#include <string>

namespace Platform
{
    inline std::wstring currentCommandLine()
    {
        return ("\"" + QApplication::applicationFilePath().replace('/', '\\') + "\" -P \"" +
                Settings::getInstance().getCurrentProfile() + "\"").toStdWString();
    }

    inline std::wstring currentRegistryKeyName()
    {
        return (QString("qTox - ") + Settings::getInstance().getCurrentProfile()).toStdWString();
    }
}

bool Platform::setAutorun(bool on)
{
    HKEY key = 0;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                    0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS)
        return false;

    bool result = false;
    std::wstring keyName = currentRegistryKeyName();

    if (on)
    {
        std::wstring path = currentCommandLine();
        result = RegSetValueEx(key, keyName.c_str(), 0, REG_SZ, (PBYTE)path.c_str(),
                               path.length() * sizeof(wchar_t)) == ERROR_SUCCESS;
    }
    else
        result = RegDeleteValue(key, keyName.c_str()) == ERROR_SUCCESS;

    RegCloseKey(key);
    return result;
}

bool Platform::getAutorun()
{
    HKEY key = 0;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                    0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS)
        return false;

    std::wstring keyName = currentRegistryKeyName();;

    wchar_t path[MAX_PATH] = { 0 };
    DWORD length = sizeof(path);
    DWORD type = REG_SZ;
    bool result = false;

    if (RegQueryValueEx(key, keyName.c_str(), 0, &type, (PBYTE)path, &length) == ERROR_SUCCESS && type == REG_SZ)
        result = true;

    RegCloseKey(key);
    return result;
}

#endif  // Q_OS_WIN32
