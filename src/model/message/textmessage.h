/*
    Copyright © 2017 by The qTox Project Contributors

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

#ifndef TEXT_MESSAGE_H
#define TEXT_MESSAGE_H

#include "usermessage.h"

#include "src/core/toxpk.h"

class TextMessage : public UserMessage
{
    Q_OBJECT
public:
    TextMessage(int id, const ToxPk& author, const QString& text, const QDateTime& time);
    TextMessage(int id, const ToxPk& author, const QString& text, const QDateTime& time,
                bool isAction);

    TextMessage(const TextMessage& message);
    TextMessage& operator=(const TextMessage& message);

    void setText(const QString& text);
    const QString& getText() const;

    bool isAction() const;
    static QString makeActionText(const QString& text);

    void setId(int id);
    int getId() const;

private:
    // TODO: Remove id (replace on pointer on message itself)
    QString text;
    int id;
    bool mIsAction;
};

#endif // TEXT_MESSAGE_H
