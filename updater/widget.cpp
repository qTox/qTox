/*
    Copyright © 2014 by The qTox Project

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
    along with qTox.  If not, see <http://www.gnu.org/licenses/>
*/


#include "widget.h"
#include "ui_widget.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QMetaObject>
#include <QDebug>
#include <QSettings>

#include "update.h"

#ifdef Q_OS_WIN
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600 // Vista for SHGetKnownFolderPath
#include <windows.h>
#include <shldisp.h>
#include <shlobj.h>
#include <exdisp.h>

const bool supported = true;
const QString QTOX_PATH = "qtox.exe";
#else
const bool supported = false;
const QString QTOX_PATH;
#endif
const QString SETTINGS_FILE = "settings.ini";

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    // Updates only for supported platforms
    if (!supported)
        fatalError(tr("The qTox updater is not supported on this platform."));

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
    HANDLE hProcessToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hProcessToken))
        goto unelevateFail;
    TOKEN_PRIVILEGES tkp;
    tkp.PrivilegeCount = 1;
    LookupPrivilegeValueW(NULL, SE_INCREASE_QUOTA_NAME, &tkp.Privileges[0].Luid);
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hProcessToken, FALSE, &tkp, 0, NULL, NULL);
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
    // Based on experimentation, this is the minimal set of rights required for CreateProcessWithTokenW (contrary to current documentation).
    if (!DuplicateTokenEx(hShellProcessToken, dwTokenRights, NULL, SecurityImpersonation, TokenPrimary, &hPrimaryToken))
        goto unelevateFail;

    qDebug() << "Unelevated primary access token acquired";
    goto unelevateCleanup;
unelevateFail:
    qWarning() << "Unelevate failed, couldn't get access token";
unelevateCleanup:
    CloseHandle(hShellProcessToken);
    CloseHandle(hShellProcess);
#endif

    QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
}

Widget::~Widget()
{
#ifdef Q_OS_WIN
    CloseHandle(hPrimaryToken);
#endif
    delete ui;
}

void Widget::setProgress(int value)
{
    ui->progress->setValue(value);
    ui->progress->repaint();
    qApp->processEvents();
}

void Widget::fatalError(QString message)
{
    qCritical() << "Update aborted with error:"<<message;
    QMessageBox::critical(this,tr("Error"), message+'\n'+tr("qTox will restart now."));
    deleteUpdate();
    restoreBackups();
    startQToxAndExit();
}

void Widget::deleteUpdate()
{
    QDir updateDir(getSettingsDirPath()+"/update/");
    updateDir.removeRecursively();
}

void Widget::startQToxAndExit()
{
#ifdef Q_OS_WIN
    // Try to restart qTox as the actual user with our unelevated token
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    SecureZeroMemory(&si, sizeof(si));
    SecureZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    bool unelevateOk = true;

    auto advapi32H = LoadLibrary(TEXT("advapi32.dll"));
    if ((unelevateOk = (advapi32H != nullptr)))
    {
        auto CreateProcessWithTokenWH = (decltype(&CreateProcessWithTokenW))
                                        GetProcAddress(advapi32H, "CreateProcessWithTokenW");
        if ((unelevateOk = (CreateProcessWithTokenWH != nullptr)))
        {
            if (!CreateProcessWithTokenWH(hPrimaryToken, 0, QTOX_PATH.toStdWString().c_str(), 0, 0, 0, 0, &si, &pi))
                unelevateOk = false;
        }
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (!unelevateOk)
    {
        qWarning() << "Failed to start unelevated qTox";
        QProcess::startDetached(QTOX_PATH);
    }

#else
    QProcess::startDetached(QTOX_PATH);
#endif
    exit(0);
}

void Widget::deleteBackups()
{
    for (QString file : backups)
        QFile(file+".bak").remove();
}

void Widget::restoreBackups()
{
    for (QString file : backups)
        QFile(file+".bak").rename(file);
}

QString Widget::getSettingsDirPath()
{
    if (isToxPortableEnabled())
        return ".";

#ifdef Q_OS_WIN
    wchar_t* path;
    wchar_t pathOld[MAX_PATH];
    bool isOld = false; // If true, we have to use pathOld and older Windows API.

    auto shell32H = LoadLibrary(TEXT("shell32.dll"));
    if (!(isOld = (shell32H == nullptr)))
    {
        auto SHGetKnownFolderPathH = (decltype(&SHGetKnownFolderPath))
                                        GetProcAddress(shell32H, "SHGetKnownFolderPath");
        if (!(isOld = (SHGetKnownFolderPathH == nullptr)))
            SHGetKnownFolderPathH(FOLDERID_RoamingAppData, 0, hPrimaryToken, &path);
    }
    if (isOld)
    {
        qDebug() << "Falling back to legacy APIs...";
        SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, pathOld);
    }

    QString pathStr = QString::fromStdWString(isOld ? pathOld : path);
    pathStr.replace("\\", "/");
    return pathStr + "/tox";
#else
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QDir::separator() + "tox");
#endif
}

bool Widget::isToxPortableEnabled()
{
    QFile portableSettings(SETTINGS_FILE);
    if (portableSettings.exists())
    {
        QSettings ps(SETTINGS_FILE, QSettings::IniFormat);
        ps.beginGroup("General");
        return ps.value("makeToxPortable", false).toBool();
    }
    else
    {
        return false;
    }
}

void Widget::update()
{
    /// 1. Find and parse the update (0-5%)
    // Check that the dir exists
    QString updateDirStr = getSettingsDirPath()+"/update/";
    QDir updateDir(updateDirStr);
    if (!updateDir.exists())
        fatalError(tr("No update found."));

    setProgress(2);

    // Check that we have a flist and that every file on the diff exists
    QFile updateFlistFile(updateDirStr+"flist");
    if (!updateFlistFile.open(QIODevice::ReadOnly))
        fatalError(tr("The update is incomplete."));

    QByteArray updateFlistData = updateFlistFile.readAll();
    updateFlistFile.close();

    QList<UpdateFileMeta> updateFlist = parseFlist(updateFlistData);
    setProgress(5);

    /// 2. Generate a diff (5-50%)
    QList<UpdateFileMeta> diff = genUpdateDiff(updateFlist, this);
    for (UpdateFileMeta fileMeta : diff)
        if (!QFile::exists(updateDirStr+fileMeta.installpath))
            fatalError(tr("The update is incomplete."));

    if (diff.size() == 0)
       fatalError(tr("The update is empty!"));
    setProgress(50);
    qDebug() << "Diff generated,"<<diff.size()<<"files to update";

    /// 2. Check the update (50-75%)
    float checkProgressStep = 25.0/(float)diff.size();
    float checkProgress = 50;
    for (UpdateFileMeta fileMeta : diff)
    {
        UpdateFile file;
        file.metadata = fileMeta;

        QFile fileFile(updateDirStr+fileMeta.installpath);
        if (!fileFile.open(QIODevice::ReadOnly))
            fatalError(tr("Update files are unreadable."));

        file.data = fileFile.readAll();
        fileFile.close();

        if (file.data.size() != (int)fileMeta.size)
            fatalError(tr("Update files are corrupted."));

        if (crypto_sign_verify_detached(file.metadata.sig, (unsigned char*)file.data.data(),
                                        file.data.size(), key) != 0)
            fatalError(tr("Update files are corrupted."));

        checkProgress += checkProgressStep;
        setProgress(checkProgress);
    }
    setProgress(75);
    qDebug() << "Update files signature verified, installing";

    /// 3. Install the update (75-95%)
    float installProgressStep = 20.0/(float)diff.size();
    float installProgress = 75;
    for (UpdateFileMeta fileMeta : diff)
    {
        // Backup old files
        if (QFile(fileMeta.installpath).exists())
        {
            QFile(fileMeta.installpath+".bak").remove();
            QFile(fileMeta.installpath).rename(fileMeta.installpath+".bak");
            backups.append(fileMeta.installpath);
        }

        // Install new ones
        QDir().mkpath(QFileInfo(fileMeta.installpath).absolutePath());
        QFile fileFile(updateDirStr+fileMeta.installpath);
        if (!fileFile.copy(fileMeta.installpath))
            fatalError(tr("Unable to copy the update's files from ")+(updateDirStr+fileMeta.installpath)+" to "+fileMeta.installpath);
        installProgress += installProgressStep;
        setProgress(installProgress);
    }
    setProgress(95);

    /// 4. Delete the update and backups (95-100%)
    deleteUpdate();
    setProgress(97);
    deleteBackups();
    setProgress(100);

    /// 5. Start qTox and exit
    qDebug() << "Update applied, restarting qTox!";
    startQToxAndExit();
}
