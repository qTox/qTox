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

#include "chatareawidget.h"
#include <QAbstractTextDocumentLayout>
#include <QMessageBox>
#include <QScrollBar>

ChatAreaWidget::ChatAreaWidget(QWidget *parent) :
    QTextEdit(parent)
{
    setReadOnly(true);
    viewport()->setCursor(Qt::ArrowCursor);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setUndoRedoEnabled(false);
}

ChatAreaWidget::~ChatAreaWidget()
{
    for (ChatAction *it : messages)
        delete it;
}

void ChatAreaWidget::mouseReleaseEvent(QMouseEvent * event)
{
    QTextEdit::mouseReleaseEvent(event);
    int pos = this->document()->documentLayout()->hitTest(event->pos(), Qt::ExactHit);
    if (pos > 0)
    {
        QTextCursor cursor(document());
            cursor.setPosition(pos);
        if( ! cursor.atEnd() )
        {
            cursor.setPosition(pos+1);

            QTextFormat format = cursor.charFormat();
            if (format.isImageFormat())
            {
                QString imageName = format.toImageFormat().name();
                if (QRegExp("^data:ftrans.*").exactMatch(imageName))
                {
                    QString data = imageName.right(imageName.length() - 12);
                    int endpos = data.indexOf("/png;base64");
                    data = data.left(endpos);
                    int middlepos = data.indexOf(".");
                    QString widgetID = data.left(middlepos);
                    QString widgetBtn = data.right(data.length() - middlepos - 1);
                    emit onFileTranfertInterract(widgetID, widgetBtn);
                }
            }
        }
    }
}

QString ChatAreaWidget::getHtmledMessages()
{
    QString res("<table width=100%>\n");

    for (ChatAction *it : messages)
    {
        res += it->getHtml();
    }
    res += "</table>";
    return res;
}

void ChatAreaWidget::insertMessage(ChatAction* msgAction)
{
    if (msgAction == nullptr)
        return;

    messages.append(msgAction);
    //updateChatContent();

    moveCursor(QTextCursor::End);
    moveCursor(QTextCursor::PreviousCell);
    insertHtml(msgAction->getHtml());

    //delete msgAction;
}

void ChatAreaWidget::updateChatContent()
{
    QScrollBar* scroll = verticalScrollBar();
    lockSliderToBottom = scroll && scroll->value() == scroll->maximum();

    setUpdatesEnabled(false);
    setHtml(getHtmledMessages());
    setUpdatesEnabled(true);
    if (lockSliderToBottom)
        sliderPosition = scroll->maximum();

    scroll->setValue(sliderPosition);
}

void ChatAreaWidget::clearMessages()
{
    for (ChatAction *it : messages)
        delete it;
    updateChatContent();
}
