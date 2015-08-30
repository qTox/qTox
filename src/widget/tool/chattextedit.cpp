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

#include "chattextedit.h"
#include "src/widget/translator.h"
#include <QKeyEvent>
#include <QMenu>

ChatTextEdit::ChatTextEdit(QWidget *parent) :
    QTextEdit(parent)
{
    retranslateUi();
    setAcceptRichText(false);

    Translator::registerHandler(std::bind(&ChatTextEdit::retranslateUi, this), this);
}

ChatTextEdit::~ChatTextEdit()
{
    Translator::unregister(this);
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

void ChatTextEdit::retranslateUi()
{
    setPlaceholderText(tr("Type your message here..."));
}

void ChatTextEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = QTextEdit::createStandardContextMenu();
    if (menu == NULL)
        return QTextEdit::contextMenuEvent(e);

    QList<QAction*> actions = menu->actions();

    foreach (QAction *action, actions)
    {
       QString actionText = action->text();
       if (actionText.contains("Undo"))
       {
           actionText.replace("Undo", tr("Undo"));
           action->setText(actionText);
       }
       else if (actionText.contains("Redo"))
       {
           actionText.replace("Redo", tr("Redo"));
           action->setText(actionText);
       }
       else if (actionText.contains("Cu&t"))
       {
           actionText.replace("Cu&t", tr("Cut"));
           action->setText(actionText);
       }
       else if (actionText.contains("Copy"))
       {
           actionText.replace("Copy", tr("Copy"));
           action->setText(actionText);
       }
       else if (actionText.contains("Paste"))
       {
           actionText.replace("Paste", tr("Paste"));
           action->setText(actionText);
       }
       else if (actionText.contains("Delete"))
       {
           actionText.replace("Delete", tr("Delete"));
           action->setText(actionText);
       }
       else if (actionText.contains("Select All"))
       {
           actionText.replace("Select All", tr("Select All"));
           action->setText(actionText);
       }
    }

    menu->exec(e->globalPos());
}
