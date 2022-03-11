/*
    Copyright © 2005-2014 by the Quassel Project
    devel@quassel-irc.org

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

#pragma once

#include "src/model/group.h"
#include "src/widget/tool/chattextedit.h"
#include <QMap>
#include <QString>

class TabCompleter : public QObject
{
    Q_OBJECT
public:
    TabCompleter(ChatTextEdit* msgEdit_, Group* group_);

public slots:
    void complete();
    void reset();

private:
    struct SortableString
    {
        explicit SortableString(const QString& n)
            : contents{n}
        {
        }
        bool operator<(const SortableString& other) const;
        QString contents;
    };

    ChatTextEdit* msgEdit;
    Group* group;
    bool enabled;
    const static QString nickSuffix;

    QMap<SortableString, QString> completionMap;
    QMap<SortableString, QString>::Iterator nextCompletion;
    int lastCompletionLength;

    void buildCompletionList();
};
