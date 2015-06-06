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

#define SMILEYPACK_SEARCH_PATHS                                                                                             \
    {                                                                                                                       \
        ":/smileys", "./smileys", "/usr/share/qtox/smileys", "/usr/share/emoticons", "~/.kde4/share/emoticons", "~/.kde/share/emoticons" \
    }

//maps emoticons to smileys
class SmileyPack : public QObject
{
    Q_OBJECT
public:
    static SmileyPack& getInstance();
    static QList<QPair<QString, QString> > listSmileyPacks(const QStringList& paths = SMILEYPACK_SEARCH_PATHS);
    static bool isValid(const QString& filename);

    bool load(const QString& filename);
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

    void cacheSmiley(const QString& name);
    QIcon getCachedSmiley(const QString& key);

    QHash<QString, QString> filenameTable; // matches an emoticon to its corresponding smiley ie. ":)" -> "happy.png"
    QHash<QString, QIcon> iconCache; // representation of a smiley ie. "happy.png" -> data
    QList<QStringList> emoticons; // {{ ":)", ":-)" }, {":(", ...}, ... }
    QString path; // directory containing the cfg and image files
};

#endif // SMILEYPACK_H
