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
#include "src/net/updatecheck.h"
#include "src/persistence/settings.h"

#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include <QDebug>
#include <cassert>

namespace {
    const QString versionUrl{QStringLiteral("https://api.github.com/repos/qTox/qTox/releases/latest")};
} // namespace

UpdateCheck::UpdateCheck(const Settings& settings)
    : settings(settings)
{
    updateTimer.start(1000 * 60 * 60 * 24 /* 1 day */);
    connect(&updateTimer, &QTimer::timeout, this, &UpdateCheck::checkForUpdate);
    connect(&manager, &QNetworkAccessManager::finished, this, &UpdateCheck::handleResponse);
}

void UpdateCheck::checkForUpdate()
{
    if (!settings.getCheckUpdates()) {
        // still run the timer to check periodically incase setting changes
        return;
    }
    manager.setProxy(settings.getProxy());
    QNetworkRequest request{versionUrl};
    manager.get(request);
}

void UpdateCheck::handleResponse(QNetworkReply *reply)
{
    assert(reply != nullptr);
    if (reply == nullptr) {
        qWarning() << "Update check returned null reply, ignoring";
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Failed to check for update:" << reply->error();
        emit updateCheckFailed();
        reply->deleteLater();
        return;
    }
    QByteArray result = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(result);
    QJsonObject jObject = doc.object();
    QVariantMap mainMap = jObject.toVariantMap();
    QString latestVersion = mainMap["tag_name"].toString();
    if (latestVersion.isEmpty()) {
        qWarning() << "No tag name found in response:";
        emit updateCheckFailed();
        reply->deleteLater();
        return;
    }

    // capture tag name to avoid showing update available on dev builds which include hash as part of describe
    QRegularExpression versionFormat{QStringLiteral("v[0-9]+.[0-9]+.[0-9]+")};
    QString curVer = versionFormat.match(GIT_DESCRIBE).captured(0);
    if (latestVersion != curVer) {
        qInfo() << "Update available to version" << latestVersion;
        QUrl link{mainMap["html_url"].toString()};
        emit updateAvailable(latestVersion, link);
    }
    else {
        qInfo() << "qTox is up to date";
        emit upToDate();
    }
    reply->deleteLater();
}
