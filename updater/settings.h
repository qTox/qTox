#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class Settings
{
public:
    Settings();
    ~Settings();

    QString getSettingsDirPath() const; ///< The returned path ends with a directory separator
#ifdef Q_OS_WIN
    HANDLE getPrimaryToken() const; ///< Used to impersonnate the unelevated user
#endif

private:
    bool portable;
    static constexpr const char* SETTINGS_FILE = "qtox.ini";
#ifdef Q_OS_WIN
    HANDLE hPrimaryToken;
#endif
};

#endif // SETTINGS_H
