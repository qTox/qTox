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

#pragma once

#include <QIcon>
#include <QMap>
#include <QMutex>
#include <QRegularExpression>

#include <memory>

class QTimer;
class ISmileySettings;

class SmileyPack : public QObject
{
    Q_OBJECT

public:
    explicit SmileyPack(ISmileySettings& settings);
    SmileyPack(SmileyPack&) = delete;
    SmileyPack& operator=(const SmileyPack&) = delete;
    ~SmileyPack() override;

    static QList<QPair<QString, QString>> listSmileyPacks(const QStringList& paths);
    static QList<QPair<QString, QString>> listSmileyPacks();

    QString smileyfied(const QString& msg);
    QList<QStringList> getEmoticons() const;
    std::shared_ptr<QIcon> getAsIcon(const QString& emoticon) const;
    static QString getAsRichText(const QString& key);

private slots:
    void onSmileyPackChanged();
    void cleanupIconsCache();

private:
    bool load(const QString& filename);
    void constructRegex();

    mutable std::map<QString, std::shared_ptr<QIcon>> cachedIcon;
    QHash<QString, QString> emoticonToPath;
    QList<QStringList> emoticons;
    QString path;
    QTimer* cleanupTimer;
    QRegularExpression smilify;
    mutable QMutex loadingMutex;
    ISmileySettings& settings;
};
