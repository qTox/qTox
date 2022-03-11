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

#include "customtextdocument.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/style.h"

#include <QDebug>
#include <QIcon>
#include <QUrl>

CustomTextDocument::CustomTextDocument(SmileyPack& smileyPack_,
    Settings& settings_, QObject* parent)
    : QTextDocument(parent)
    , smileyPack(smileyPack_)
    , settings(settings_)
{
    setUndoRedoEnabled(false);
    setUseDesignMetrics(false);
}

QVariant CustomTextDocument::loadResource(int type, const QUrl& name)
{
    if (type == QTextDocument::ImageResource && name.scheme() == "key") {
        QSize size = QSize(settings.getEmojiFontPointSize(),
                           settings.getEmojiFontPointSize());
        QString fileName = QUrl::fromPercentEncoding(name.toEncoded()).mid(4).toHtmlEscaped();

        std::shared_ptr<QIcon> icon = smileyPack.getAsIcon(fileName);
        emoticonIcons.append(icon);
        return icon->pixmap(size);
    }

    return QTextDocument::loadResource(type, name);
}
