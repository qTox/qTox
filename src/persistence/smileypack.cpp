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

#include "smileypack.h"
#include "src/persistence/settings.h"
#include "src/widget/style.h"

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
#include <QtConcurrent/QtConcurrentRun>

/**
@class SmileyPack
@brief Maps emoticons to smileys.

@var QHash<QString, QString> SmileyPack::filenameTable
@brief Matches an emoticon to its corresponding smiley ie. ":)" -> "happy.png"

@var QHash<QString, QIcon> SmileyPack::iconCache
@brief representation of a smiley ie. "happy.png" -> data

@var QList<QStringList> SmileyPack::emoticons
@brief {{ ":)", ":-)" }, {":(", ...}, ... }

@var QString SmileyPack::path
@brief directory containing the cfg and image files
*/

SmileyPack::SmileyPack()
{
    loadingMutex.lock();
    QtConcurrent::run(this, &SmileyPack::load, Settings::getInstance().getSmileyPack());
    connect(&Settings::getInstance(), &Settings::smileyPackChanged, this, &SmileyPack::onSmileyPackChanged);
}

/**
@brief Returns the singleton instance.
*/
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
                    if (!smileyPacks.contains(QPair<QString, QString>(packageName, absPath)))
                        smileyPacks << QPair<QString, QString>(packageName, absPath);
                    else if (!smileyPacks.contains(QPair<QString, QString>(packageName, relPath)))
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

/**
@brief Load smile pack
@note The caller must lock loadingMutex and should run it in a thread
@param filename Filename of smilepack.
@return False if cannot open file, true otherwise.
*/
bool SmileyPack::load(const QString& filename)
{
    // discard old data
    filenameTable.clear();
    iconCache.clear();
    emoticons.clear();
    path.clear();

    // open emoticons.xml
    QFile xmlFile(filename);
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        loadingMutex.unlock();
        return false; // cannot open file
    }

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
            QString emoticon = stringElement.text()
                                .replace("<","&lt;").replace(">","&gt;");
            filenameTable.insert(emoticon, file);

            cacheSmiley(file); // preload all smileys

            if (!getCachedSmiley(emoticon).isNull())
                emoticonSet.push_back(emoticon);

            stringElement = stringElement.nextSibling().toElement();

        }

        if (emoticonSet.size() > 0)
            emoticons.push_back(emoticonSet);
    }

    // success!
    loadingMutex.unlock();
    return true;
}

QString SmileyPack::smileyfied(QString msg)
{
    QMutexLocker locker(&loadingMutex);

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
    QMutexLocker locker(&loadingMutex);
    return emoticons;
}

QString SmileyPack::getAsRichText(const QString &key)
{
    return QString("<img title=\"%1\" src=\"key:%1\"\\>").arg(key);
}

QIcon SmileyPack::getAsIcon(const QString &key)
{
    QMutexLocker locker(&loadingMutex);
    return getCachedSmiley(key);
}

void SmileyPack::cacheSmiley(const QString &name)
{
    QString filename = QDir(path).filePath(name);

    QIcon icon;
    icon.addFile(filename);
    iconCache.insert(name, icon);
}

QIcon SmileyPack::getCachedSmiley(const QString &key)
{
    // valid key?
    if (!filenameTable.contains(key))
        return QPixmap();

    // cache it if needed
    QString file = filenameTable.value(key);
    if (!iconCache.contains(file))
        cacheSmiley(file);

    return iconCache.value(file);
}

void SmileyPack::onSmileyPackChanged()
{
    loadingMutex.lock();
    QtConcurrent::run(this, &SmileyPack::load, Settings::getInstance().getSmileyPack());
}
