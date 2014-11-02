/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

    This file is part of qTox, a Qt-based graphical interface for Tox.

    This program is libre software: you can redistribute it and/or modify
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
#include "style.h"

#include <QFileInfo>
#include <QFile>
#include <QFont>
#include <QFontInfo>
#include <QIcon>
#include <QPixmap>
#include <QDir>
#include <QCoreApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QBuffer>
#include <QStringBuilder>

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

QList<QPair<QString, QString> > SmileyPack::listSmileyPacks(const QStringList &paths)
{
    QList<QPair<QString, QString> > smileyPacks;

    for (QString path : paths)
    {
        if (path.leftRef(1) == "~")
            path.replace(0, 1, QDir::homePath());

        QDir dir(path);
        if (!dir.exists())
            continue;

        for (const QString& subdirectory : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        {
            dir.cd(subdirectory);

            QFileInfoList entries = dir.entryInfoList(QStringList() << "emoticons.xml", QDir::Files);
            if (entries.size() > 0) // does it contain a file called emoticons.xml?
            {
                QString packageName = dir.dirName();
                QString absPath = entries[0].absoluteFilePath();
                QString relPath = QDir(QCoreApplication::applicationDirPath()).relativeFilePath(absPath);

                if (relPath.leftRef(2) == "..")
                {
                    if(!smileyPacks.contains(QPair<QString, QString>(packageName, absPath)))
                        smileyPacks << QPair<QString, QString>(packageName, absPath);
                    else if(!smileyPacks.contains(QPair<QString, QString>(packageName, relPath)))
                        smileyPacks << QPair<QString, QString>(packageName, relPath); // use relative path for subdirectories                            
                }
            }
            dir.cdUp();
        }
    }

    return smileyPacks;
}

bool SmileyPack::isValid(const QString &filename)
{
    return QFile(filename).exists();
}

bool SmileyPack::load(const QString& filename)
{
    // discard old data
    filenameTable.clear();
    imgCache.clear();
    emoticons.clear();
    path.clear();

    // open emoticons.xml
    QFile xmlFile(filename);
    if(!xmlFile.open(QIODevice::ReadOnly))
        return false; // cannot open file

    /* parse the cfg file
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

    path = QFileInfo(filename).absolutePath();

    QDomDocument doc;
    doc.setContent(xmlFile.readAll());

    QDomNodeList emoticonElements = doc.elementsByTagName("emoticon");
    for (int i = 0; i < emoticonElements.size(); ++i)
    {
        QString file = emoticonElements.at(i).attributes().namedItem("file").nodeValue();
        QDomElement stringElement = emoticonElements.at(i).firstChildElement("string");

        QStringList emoticonSet; // { ":)", ":-)" } etc.

        while (!stringElement.isNull())
        {
            QString emoticon = stringElement.text();
            filenameTable.insert(emoticon, file);
            
            cacheSmiley(file); // preload all smileys
            
            QPixmap pm;
            pm.loadFromData(getCachedSmiley(emoticon), "PNG");
            
            if(pm.size().width() > 0) 
                emoticonSet.push_back(emoticon);
            
            stringElement = stringElement.nextSibling().toElement();
            
        }
        
        if(emoticonSet.size() > 0)
            emoticons.push_back(emoticonSet);
    }

    // success!
    return true;
}

QString SmileyPack::smileyfied(QString msg)
{
    QRegExp exp("\\S+"); // matches words

    int index = msg.indexOf(exp);

    // if a word is key of a smiley, replace it by its corresponding image in Rich Text
    while (index >= 0)
    {
        QString key = exp.cap();
        if (filenameTable.contains(key))
        {
            QString imgRichText = getAsRichText(key);

            msg.replace(index, key.length(), imgRichText);
            index += imgRichText.length() - key.length();
        }
        index = msg.indexOf(exp, index + key.length());
    }

    return msg;
}

QList<QStringList> SmileyPack::getEmoticons() const
{
    return emoticons;
}

QString SmileyPack::getAsRichText(const QString &key)
{
    return "<img title=\""%key%"\" src=\"data:image/png;base64," % QString(getCachedSmiley(key).toBase64()) % "\">";
}

QIcon SmileyPack::getAsIcon(const QString &key)
{
    QPixmap pm;
    pm.loadFromData(getCachedSmiley(key), "PNG");
    return QIcon(pm);
}

void SmileyPack::cacheSmiley(const QString &name)
{
    // The -1 is to avoid having the space for descenders under images move the text down
    // We can't remove it because Qt doesn't support CSS display or vertical-align
    int fontHeight = QFontInfo(Style::getFont(Style::Big)).pixelSize() - 1;
    QSize size(fontHeight, fontHeight);
    QString filename = QDir(path).filePath(name);
    QImage img(filename);

    if (!img.isNull())
    {
        QImage scaledImg = img.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        QByteArray scaledImgData;
        QBuffer buffer(&scaledImgData);
        scaledImg.save(&buffer, "PNG");

        imgCache.insert(name, scaledImgData);
    }
}

QByteArray SmileyPack::getCachedSmiley(const QString &key)
{
    // valid key?
    if (!filenameTable.contains(key))
        return QByteArray();

    // cache it if needed
    QString file = filenameTable.value(key);
    if (!imgCache.contains(file)) {
        cacheSmiley(file);
    }

    return imgCache.value(file);
}

void SmileyPack::onSmileyPackChanged()
{
    load(Settings::getInstance().getSmileyPack());
}
