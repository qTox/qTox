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
#include <QString>
#include <QObject>

//maps emoticons to smileys
class SmileyPack : public QObject
{
    Q_OBJECT
public:
    static SmileyPack& getInstance();

    bool load(const QString &filename);
    QString replaceEmoticons(const QString& msg);

private slots:
    void onSmileyPackChanged();

private:
    SmileyPack();
    SmileyPack(SmileyPack&) = delete;
    SmileyPack& operator=(const SmileyPack&) = delete;

    void loadSmiley(const QString& name);

    QHash<QString, QString> assignmentTable; // matches an emoticon to its corresponding smiley
    QHash<QString, QString> cache;
    QString path; // directory containing the cfg file
};

#endif // SMILEYPACK_H
