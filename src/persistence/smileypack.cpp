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

#include "smileypack.h"
#include "src/persistence/settings.h"

#include <QDir>
#include <QDomElement>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QtConcurrent/QtConcurrentRun>
#include <QTimer>

#if defined(Q_OS_FREEBSD)
#include <locale.h>
#endif

/**
 * @class SmileyPack
 * @brief Maps emoticons to smileys.
 *
 * @var SmileyPack::filenameTable
 * @brief Matches an emoticon to its corresponding smiley ie. ":)" -> "happy.png"
 *
 * @var SmileyPack::iconCache
 * @brief representation of a smiley ie. "happy.png" -> data
 *
 * @var SmileyPack::emoticons
 * @brief {{ ":)", ":-)" }, {":(", ...}, ... }
 *
 * @var SmileyPack::path
 * @brief directory containing the cfg and image files
 *
 * @var SmileyPack::defaultPaths
 * @brief Contains all directories where smileys could be found
 */

QStringList loadDefaultPaths();

static const QStringList DEFAULT_PATHS = loadDefaultPaths();

static const QString RICH_TEXT_PATTERN = QStringLiteral("<img title=\"%1\" src=\"key:%1\"\\>");

static const QString EMOTICONS_FILE_NAME = QStringLiteral("emoticons.xml");

static constexpr int CLEANUP_TIMEOUT = 5 * 60 * 1000; // 5 minutes

/**
 * @brief Construct list of standard directories with "emoticons" sub dir, whether these directories
 * exist or not
 * @return Constructed list of default emoticons directories
 */
QStringList loadDefaultPaths()
{
#if defined(Q_OS_FREEBSD)
    // TODO: Remove when will be fixed.
    // Workaround to fix https://bugreports.qt.io/browse/QTBUG-57522
    setlocale(LC_ALL, "");
#endif
    const QString EMOTICONS_SUB_PATH = QDir::separator() + QStringLiteral("emoticons");
    QStringList paths{":/smileys", "~/.kde4/share/emoticons", "~/.kde/share/emoticons",
                      EMOTICONS_SUB_PATH};

    // qTox exclusive emoticons
    QStandardPaths::StandardLocation location;
    location = QStandardPaths::AppDataLocation;

    QStringList locations = QStandardPaths::standardLocations(location);
    // system wide emoticons
    locations.append(QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation));
    for (QString qtoxPath : locations) {
        qtoxPath.append(EMOTICONS_SUB_PATH);
        if (!paths.contains(qtoxPath)) {
            paths.append(qtoxPath);
        }
    }

    return paths;
}

/**
 * @brief Wraps passed string into smiley HTML image reference
 * @param key Describes which smiley is needed
 * @return Key that wrapped into image ref
 */
QString getAsRichText(const QString& key)
{
    return RICH_TEXT_PATTERN.arg(key);
}

SmileyPack::SmileyPack()
    : cleanupTimer{new QTimer(this)}
{
    loadingMutex.lock();
    QtConcurrent::run(this, &SmileyPack::load, Settings::getInstance().getSmileyPack());
    connect(&Settings::getInstance(), &Settings::smileyPackChanged, this,
            &SmileyPack::onSmileyPackChanged);
    connect(cleanupTimer, &QTimer::timeout, this, &SmileyPack::cleanupIconsCache);
    cleanupTimer->start(CLEANUP_TIMEOUT);
}

SmileyPack::~SmileyPack()
{
    delete cleanupTimer;
}

void SmileyPack::cleanupIconsCache()
{
    QMutexLocker locker(&loadingMutex);
    for (auto it = cachedIcon.begin(); it != cachedIcon.end();) {
        std::shared_ptr<QIcon>& icon = it->second;
        if (icon.use_count() == 1) {
            it = cachedIcon.erase(it);
            icon.reset();
        } else {
            ++it;
        }
    }
}

/**
 * @brief Returns the singleton instance.
 */
SmileyPack& SmileyPack::getInstance()
{
    static SmileyPack smileyPack;
    return smileyPack;
}

/**
 * @brief Does the same as listSmileyPaths, but with default paths
 */
QList<QPair<QString, QString>> SmileyPack::listSmileyPacks()
{
    return listSmileyPacks(DEFAULT_PATHS);
}

/**
 * @brief Searches all files called "emoticons.xml" within the every passed path in the depth of 2
 * @param paths Paths where to search for file
 * @return Vector of pairs: {directoryName, absolutePathToFile}
 */
QList<QPair<QString, QString>> SmileyPack::listSmileyPacks(const QStringList& paths)
{
    QList<QPair<QString, QString>> smileyPacks;
    const QString homePath = QDir::homePath();
    for (QString path : paths) {
        if (path.startsWith('~')) {
            path.replace(0, 1, homePath);
        }

        QDir dir(path);
        if (!dir.exists()) {
            continue;
        }

        for (const QString& subdirectory : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            dir.cd(subdirectory);
            if (dir.exists(EMOTICONS_FILE_NAME)) {
                QString absPath = dir.absolutePath() + QDir::separator() + EMOTICONS_FILE_NAME;
                QPair<QString, QString> p{dir.dirName(), absPath};
                if (!smileyPacks.contains(p)) {
                    smileyPacks.append(p);
                }
            }

            dir.cdUp();
        }
    }

    return smileyPacks;
}

/**
 * @brief Load smile pack
 * @note The caller must lock loadingMutex and should run it in a thread
 * @param filename Filename of smilepack.
 * @return False if cannot open file, true otherwise.
 */
bool SmileyPack::load(const QString& filename)
{
    QFile xmlFile(filename);
    if (!xmlFile.exists() || !xmlFile.open(QIODevice::ReadOnly)) {
        loadingMutex.unlock();
        return false;
    }

    QDomDocument doc;
    doc.setContent(xmlFile.readAll());
    xmlFile.close();

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
    QDomNodeList emoticonElements = doc.elementsByTagName("emoticon");
    const QString itemName = QStringLiteral("file");
    const QString childName = QStringLiteral("string");
    const int iconsCount = emoticonElements.size();
    emoticons.clear();
    emoticonToPath.clear();
    cachedIcon.clear();

    for (int i = 0; i < iconsCount; ++i) {
        QDomNode node = emoticonElements.at(i);
        QString iconName = node.attributes().namedItem(itemName).nodeValue();
        QString iconPath = QDir{path}.filePath(iconName);
        QDomElement stringElement = node.firstChildElement(childName);
        QStringList emoticonList;
        while (!stringElement.isNull()) {
            QString emoticon = stringElement.text().replace("<", "&lt;").replace(">", "&gt;");
            emoticonToPath.insert(emoticon, iconPath);
            emoticonList.append(emoticon);
            stringElement = stringElement.nextSibling().toElement();
        }

        emoticons.append(emoticonList);
    }

    loadingMutex.unlock();
    return true;
}

/**
 * @brief Replaces all found text emoticons to HTML reference with its according icon filename
 * @param msg Message where to search for emoticons
 * @return Formatted copy of message
 */
QString SmileyPack::smileyfied(const QString& msg)
{
    QMutexLocker locker(&loadingMutex);
    QString result(msg);
    for ( auto r = emoticonToPath.begin(); r != emoticonToPath.end(); ++r) {
        QRegularExpression exp;
        if (r.key().toUcs4().length() == 1) {
            // UTF-8 emoji
            exp.setPattern(r.key());
        }
        else {
            // patterns like ":)" or ":smile:", don't match inside a word or else will hit punctuation and html tags
            exp.setPattern(QStringLiteral(R"((?<=^|\s))") + QRegularExpression::escape(r.key()) + QStringLiteral(R"((?=$|\s))"));
        }
        int replaceDiff = 0;
        QRegularExpressionMatchIterator iter = exp.globalMatch(result);
        while (iter.hasNext()) {
            QRegularExpressionMatch match = iter.next();
            int startPos = match.capturedStart();
            int keyLength = r.key().length();
            QString imgRichText = getAsRichText(r.key());
            result.replace(startPos + replaceDiff, keyLength, imgRichText);
            replaceDiff += imgRichText.length() - keyLength;
        }
    }
    return result;
}

/**
 * @brief Returns all emoticons that was extracted from files, grouped by according icon file
 */
QList<QStringList> SmileyPack::getEmoticons() const
{
    QMutexLocker locker(&loadingMutex);
    return emoticons;
}

/**
 * @brief Gets icon accoring to passed emoticon
 * @param emoticon Passed emoticon
 * @return Returns cached icon according to passed emoticon, null if no icon mapped to this emoticon
 */
std::shared_ptr<QIcon> SmileyPack::getAsIcon(const QString& emoticon) const
{
    QMutexLocker locker(&loadingMutex);
    if (cachedIcon.find(emoticon) != cachedIcon.end()) {
        return cachedIcon[emoticon];
    }

    const auto iconPathIt = emoticonToPath.find(emoticon);
    if (iconPathIt == emoticonToPath.end()) {
        return std::make_shared<QIcon>();
    }

    const QString& iconPath = iconPathIt.value();
    auto icon = std::make_shared<QIcon>(iconPath);
    cachedIcon[emoticon] = icon;
    return icon;
}

void SmileyPack::onSmileyPackChanged()
{
    loadingMutex.lock();
    QtConcurrent::run(this, &SmileyPack::load, Settings::getInstance().getSmileyPack());
}
