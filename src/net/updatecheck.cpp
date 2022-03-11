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
#include "src/net/updatecheck.h"
#include "src/persistence/settings.h"

#include <QNetworkAccessManager>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QObject>
#include <QRegularExpression>
#include <QTimer>
#include <cassert>

namespace {
const QString versionUrl{QStringLiteral("https://api.github.com/repos/qTox/qTox/releases/latest")};
const QString versionRegexString{QStringLiteral("v([0-9]+)\\.([0-9]+)\\.([0-9]+)")};

struct Version {
    int major;
    int minor;
    int patch;
};

Version tagToVersion(QString tagName)
{
    // capture tag name to avoid showing update available on dev builds which include hash as part of describe
    QRegularExpression versionFormat(versionRegexString);
    auto matches = versionFormat.match(tagName);
    assert(matches.lastCapturedIndex() == 3);

    bool ok;
    auto major = matches.captured(1).toInt(&ok);
    assert(ok);
    auto minor = matches.captured(2).toInt(&ok);
    assert(ok);
    auto patch = matches.captured(3).toInt(&ok);
    assert(ok);

    return {major, minor, patch};
}

bool isUpdateAvailable(Version current, Version available)
{
    // A user may have a version greater than our latest release in the time between a tag being pushed and the release
    // being published. Don't notify about an update in that case.

    if (current.major < available.major) {
        return true;
    }
    if (current.major > available.major) {
        return false;
    }

    if (current.minor < available.minor) {
        return true;
    }
    if (current.minor > available.minor) {
        return false;
    }

    if (current.patch < available.patch) {
        return true;
    }
    if (current.patch > available.patch) {
        return false;
    }

    return false;
}

bool isCurrentVersionStable()
{
  QRegularExpression versionRegex(versionRegexString);
  auto currentVer = versionRegex.match(GIT_DESCRIBE_EXACT);
  if (currentVer.hasMatch()){
    return true;
  } else {
    return false;
  }
}

} // namespace

UpdateCheck::UpdateCheck(const Settings& settings_)
    : settings(settings_)
{
    qInfo() << "qTox is running version:" << GIT_DESCRIBE;
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

    if (isCurrentVersionStable() == false) {
        qWarning() << "Currently running an untested/unstable version of qTox";
        emit versionIsUnstable();
        return;
    }

    manager.setProxy(settings.getProxy());
    QNetworkRequest request{versionUrl};
    manager.get(request);
}

void UpdateCheck::handleResponse(QNetworkReply* reply)
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

    auto currentVer = tagToVersion(GIT_DESCRIBE);
    auto availableVer = tagToVersion(latestVersion);

    if (isUpdateAvailable(currentVer, availableVer)) {
        qInfo() << "Update available to version" << latestVersion;
        QUrl link{mainMap["html_url"].toString()};
        emit updateAvailable(latestVersion, link);
    } else {
        qInfo() << "qTox is up to date";
        emit upToDate();
    }

    reply->deleteLater();
}
