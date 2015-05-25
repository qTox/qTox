/*
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

#include "chattextedit.h"
#include <QKeyEvent>

ChatTextEdit::ChatTextEdit(QWidget *parent) :
    QTextEdit(parent)
{
    setPlaceholderText(tr("Type your message here..."));
    setAcceptRichText(false);
}

void ChatTextEdit::keyPressEvent(QKeyEvent * event)
{
    int key = event->key();
    if ((key == Qt::Key_Enter || key == Qt::Key_Return) && !(event->modifiers() & Qt::ShiftModifier))
        emit enterPressed();
    else if (key == Qt::Key_Tab)
    {
        if (event->modifiers())
            event->ignore();
        else
            emit tabPressed();
    }
    else if (key == Qt::Key_Up && this->toPlainText().isEmpty())
    {
        this->setText(lastMessage);
        this->setFocus();
        this->moveCursor(QTextCursor::MoveOperation::End,QTextCursor::MoveMode::MoveAnchor);
    }
    else
    {
        emit keyPressed();
        QTextEdit::keyPressEvent(event);
    }
}

void ChatTextEdit::setLastMessage(QString lm)
{
    lastMessage = lm;
}
