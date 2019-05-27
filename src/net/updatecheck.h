/*
    Copyright © 2014-2018 by The qTox Project Contributors

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
#include <QObject>
#include <QNetworkAccessManager>
#include <QTimer>

#ifndef APPIMAGE_UPDATER_BRIDGE_ENABLED
#include <QNetworkAccessManager>
#else
#include <AppImageUpdaterBridge>
#include <AppImageUpdaterDialog>
#include <QScopedPointer>
#endif // APPIMAGE_UPDATER_BRIDGE_ENABLED

#include <memory>

class Settings;
class QString;
class QUrl;
class QNetworkReply;
class UpdateCheck : public QObject
{
    Q_OBJECT

public:
    UpdateCheck(const Settings& settings);

signals:
#ifndef APPIMAGE_UPDATER_BRIDGE_ENABLED
    void updateAvailable(QString latestVersion, QUrl link);
#else
    void updateAvailable();
#endif
    void upToDate();
    void updateCheckFailed();

public slots:
    void checkForUpdate();

#ifndef APPIMAGE_UPDATER_BRIDGE_ENABLED
private slots:
    void handleResponse(QNetworkReply *reply);
#else

public slots:
    void initUpdate();

private slots:
    void handleUpdate(bool);
    void handleUpdateEnd();
#endif // APPIMAGE_UPDATER_BRIDGE_ENABLED

private:
#ifndef APPIMAGE_UPDATER_BRIDGE_ENABLED
    QNetworkAccessManager manager;
#else
    AppImageUpdaterBridge::AppImageDeltaRevisioner revisioner;
    QScopedPointer<AppImageUpdaterBridge::AppImageUpdaterDialog> updateDialog;
#endif // APPIMAGE_UPDATER_BRIDGE_ENABLED
    QTimer updateTimer;
    const Settings& settings;
};
