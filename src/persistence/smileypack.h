/*
    Copyright © 2014-2015 by The qTox Project

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

#ifndef SMILEYPACK_H
#define SMILEYPACK_H

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QIcon>
#include <QMutex>

class SmileyPack : public QObject
{
    Q_OBJECT
public:
    static SmileyPack& getInstance();
    static QList<QPair<QString, QString> > listSmileyPacks(const QStringList& paths);
    static QList<QPair<QString, QString> > listSmileyPacks();
    static bool isValid(const QString& filename);

    QString smileyfied(QString msg);
    QList<QStringList> getEmoticons() const;
    QString getAsRichText(const QString& key);
    QIcon getAsIcon(const QString& key);

private slots:
    void onSmileyPackChanged();

private:
    SmileyPack();
    SmileyPack(SmileyPack&) = delete;
    SmileyPack& operator=(const SmileyPack&) = delete;

    bool load(const QString& filename);
    void cacheSmiley(const QString& name);
    QIcon getCachedSmiley(const QString& key);
    static QStringList loadDefaultPaths();

    QHash<QString, QString> filenameTable;
    QHash<QString, QIcon> iconCache;
    QList<QStringList> emoticons;
    QString path;
    static QStringList defaultPaths;
    mutable QMutex loadingMutex;
};

#endif // SMILEYPACK_H
