/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>

    This file is part of Tox Qt GUI.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the COPYING file for more details.
*/

#ifndef SMILEYPACK_H
#define SMILEYPACK_H

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

//maps emoticons to smileys
class SmileyPack : public QObject
{
    Q_OBJECT
public:
    static SmileyPack& getInstance();
    static QStringList listSmileyPacks(const QString& path);

    bool load(const QString &filename);
    QString replaceEmoticons(QString msg);
    QList<QStringList> getEmoticons() const;
    QString getSmileyAsRichText(const QString& key);
    QIcon getIcon(const QString& key);  

private slots:
    void onSmileyPackChanged();

private:
    SmileyPack();
    SmileyPack(SmileyPack&) = delete;
    SmileyPack& operator=(const SmileyPack&) = delete;

    void cacheSmiley(const QString& name);
    QByteArray getCachedSmiley(const QString& key);

    QHash<QString, QString> filenameTable; // matches an emoticon to its corresponding smiley ie. ":)" -> "happy.png"
    QHash<QString, QByteArray> cache; // (scaled) representation of a smiley ie. "happy.png" -> data
    QString path; // directory containing the cfg and image files
    QList<QStringList> emoticons;
};

#endif // SMILEYPACK_H
