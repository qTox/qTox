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

#include "smileypack.h"
#include "settings.h"

#include <QFileInfo>
#include <QFile>
#include <QtXml>
#include <QDebug>

SmileyPack::SmileyPack()
{
    load(Settings::getInstance().getSmileyPack());
    connect(&Settings::getInstance(), &Settings::smileyPackChanged, this, &SmileyPack::onSmileyPackChanged);
}

SmileyPack& SmileyPack::getInstance()
{
    static SmileyPack smileyPack;
    return smileyPack;
}

bool SmileyPack::load(const QString& filename)
{
    // discard old data
    lookupTable.clear();
    QDir::setSearchPaths("smiley", QStringList());

    // open emoticons.xml
    QFile xmlFile(filename);
    if(!xmlFile.open(QIODevice::ReadOnly))
        return false; // cannot open file

    /* parse the cfg document
     * sample:
     * <?xml version='1.0'?>
     * <messaging-emoticon-map>
     *   <emoticon file="smile.png" >
     *       <string>:)</string>
     *       <string>:-)</string>
     *   </emoticon>
     *   <emoticon file="sad.png" >
     *       <string>:(</string>
     *       <string>:-(</string>
     *   </emoticon>
     * </messaging-emoticon-map>
     */

    QDomDocument doc;
    doc.setContent(xmlFile.readAll());

    QDomNodeList emoticonElements = doc.elementsByTagName("emoticon");
    for (int i = 0; i < emoticonElements.size(); ++i)
    {
        QString file = emoticonElements.at(i).attributes().namedItem("file").nodeValue();
        QDomElement stringElement = emoticonElements.at(i).firstChildElement("string");

        while (!stringElement.isNull())
        {
            QString rune = stringElement.text();
            lookupTable.insert(rune, file); // add it to the map

            stringElement = stringElement.nextSibling().toElement();
        }
    }

    // Rich Text makes use of Qt's resource system, so
    // let Qt know about our smilies
    QFileInfo info(filename);
    QDir::setSearchPaths("smiley", QStringList() << info.absolutePath());

    // success!
    return true;
}

QString SmileyPack::replaceEmoticons(const QString &msg) const
{
    QString out = msg;
    QRegExp exp("\\S*"); // matches words

    int index = msg.indexOf(exp);
    int offset = 0;

    // if a word is key of a smiley, replace it by it's corresponding image in Rich Text
    while (index >= 0 || exp.matchedLength() > 0)
    {
        QString key = exp.cap();
        if (lookupTable.contains(key))
        {
            QString width = QString::number(16);
            QString height = QString::number(16);

            QString img = lookupTable[key];
            QString imgRt = "<img src=\"smiley:" + img + "\" width=\"" + width + "\" height=\"" + height + "\">";

            out.replace(index + offset, key.length(), imgRt);
            offset += imgRt.length() - key.length();
        }
        index = msg.indexOf(exp, index + exp.matchedLength() + 1);
    }

    return out;
}

void SmileyPack::onSmileyPackChanged()
{
    load(Settings::getInstance().getSmileyPack());
}
