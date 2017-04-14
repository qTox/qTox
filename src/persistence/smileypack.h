/*
    Copyright © 2014-2017 by The qTox Project Contributors

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

#include <QIcon>
#include <QMap>
#include <QMutex>

class SmileyPack : public QObject
{
    Q_OBJECT

public:
    static SmileyPack& getInstance();
    static QList<QPair<QString, QString>> listSmileyPacks(const QStringList& paths);
    static QList<QPair<QString, QString>> listSmileyPacks();

    QString smileyfied(const QString& msg);
    QList<QStringList> getEmoticons() const;
    QIcon getAsIcon(const QString& key);

private slots:
    void onSmileyPackChanged();

private:
    SmileyPack();
    SmileyPack(SmileyPack&) = delete;
    SmileyPack& operator=(const SmileyPack&) = delete;

    bool load(const QString& filename);

    QVector<QIcon> icons;
    QMap<QString, const QIcon*> emoticonToIcon;
    QList<QStringList> emoticons;
    QString path;
    mutable QMutex loadingMutex;
};

#endif // SMILEYPACK_H
