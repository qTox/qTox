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

#include "settings.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600 // Vista for SHGetKnownFolderPath
#include <exdisp.h>
#include <shldisp.h>
#include <shlobj.h>
#include <windows.h>
#endif

Settings::Settings()
{
    portable = false;
    QFile portableSettings(SETTINGS_FILE);
    if (portableSettings.exists()) {
        QSettings ps(SETTINGS_FILE, QSettings::IniFormat);
        ps.beginGroup("General");
        portable = ps.value("makeToxPortable", false).toBool();
    }
    qDebug() << "Portable: " << portable;

#ifdef Q_OS_WIN
    // Get a primary unelevated token of the actual user
    hPrimaryToken = nullptr;
    HANDLE hShellProcess = nullptr, hShellProcessToken = nullptr;
    const DWORD dwTokenRights = TOKEN_QUERY | TOKEN_IMPERSONATE | TOKEN_ASSIGN_PRIMARY
                                | TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID;
    DWORD dwPID = 0;
    HWND hwnd = nullptr;
    DWORD dwLastErr = 0;

    // Enable SeIncreaseQuotaPrivilege
    HANDLE hProcessToken = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hProcessToken))
        goto unelevateFail;
    TOKEN_PRIVILEGES tkp;
    tkp.PrivilegeCount = 1;
    LookupPrivilegeValueW(nullptr, SE_INCREASE_QUOTA_NAME, &tkp.Privileges[0].Luid);
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hProcessToken, FALSE, &tkp, 0, nullptr, nullptr);
    dwLastErr = GetLastError();
    CloseHandle(hProcessToken);
    if (ERROR_SUCCESS != dwLastErr)
        goto unelevateFail;

    // Get a primary copy of the desktop shell's token,
    // we're assuming the shell is running as the actual user
    hwnd = GetShellWindow();
    if (!hwnd)
        goto unelevateFail;
    GetWindowThreadProcessId(hwnd, &dwPID);
    if (!dwPID)
        goto unelevateFail;
    hShellProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPID);
    if (!hShellProcess)
        goto unelevateFail;
    if (!OpenProcessToken(hShellProcess, TOKEN_DUPLICATE, &hShellProcessToken))
        goto unelevateFail;

    // Duplicate the shell's process token to get a primary token.
    // Based on experimentation, this is the minimal set of rights required for
    // CreateProcessWithTokenW (contrary to current documentation).
    if (!DuplicateTokenEx(hShellProcessToken, dwTokenRights, nullptr, SecurityImpersonation,
                          TokenPrimary, &hPrimaryToken))
        goto unelevateFail;

    qDebug() << "Unelevated primary access token acquired";
    goto unelevateCleanup;
unelevateFail:
    qWarning() << "Unelevate failed, couldn't get access token";
unelevateCleanup:
    CloseHandle(hShellProcessToken);
    CloseHandle(hShellProcess);
#endif
}

Settings::~Settings()
{
#ifdef Q_OS_WIN
    CloseHandle(hPrimaryToken);
#endif
}

QString Settings::getSettingsDirPath() const
{
    if (portable)
        return QString(".") + QDir::separator();

// workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    wchar_t* path;
    bool isOld = false; // If true, we can't unelevate and just return the path for our current home

    auto shell32H = LoadLibrary(TEXT("shell32.dll"));
    if (!(isOld = (shell32H == nullptr))) {
        auto SHGetKnownFolderPathH =
            (decltype(&SHGetKnownFolderPath))GetProcAddress(shell32H, "SHGetKnownFolderPath");
        if (!(isOld = (SHGetKnownFolderPathH == nullptr)))
            SHGetKnownFolderPathH(FOLDERID_RoamingAppData, 0, hPrimaryToken, &path);
    }

    if (isOld) {
        return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                               + QDir::separator() + "AppData" + QDir::separator() + "Roaming"
                               + QDir::separator() + "tox" + QDir::separator());
    } else {
        QString pathStr = QString::fromStdWString(path);
        pathStr.replace("\\", "/");
        return pathStr + "/tox";
    }
#elif defined(Q_OS_OSX)
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                           + QDir::separator() + "Library" + QDir::separator()
                           + "Application Support" + QDir::separator() + "Tox")
           + QDir::separator();
#else
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                           + QDir::separator() + "tox")
           + QDir::separator();
#endif
}

#ifdef Q_OS_WIN
HANDLE Settings::getPrimaryToken() const
{
    return hPrimaryToken;
}
#endif
