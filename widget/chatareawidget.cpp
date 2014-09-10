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
#include "widget/tool/chataction.h"
#include <QAbstractTextDocumentLayout>
#include <QMessageBox>
#include <QScrollBar>
#include <QDesktopServices>

ChatAreaWidget::ChatAreaWidget(QWidget *parent) :
    QTextBrowser(parent)
{
    setReadOnly(true);
    viewport()->setCursor(Qt::ArrowCursor);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setUndoRedoEnabled(false);

    setOpenExternalLinks(false);
    setOpenLinks(false);
    setAcceptRichText(false);

    connect(this, &ChatAreaWidget::anchorClicked, this, &ChatAreaWidget::onAnchorClicked);
}

ChatAreaWidget::~ChatAreaWidget()
{
    for (ChatAction* action : messages)
        delete action;
    messages.clear();
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

void ChatAreaWidget::onAnchorClicked(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}

void ChatAreaWidget::insertMessage(ChatAction *msgAction)
{
    if (msgAction == nullptr)
        return;

    moveCursor(QTextCursor::End);
    moveCursor(QTextCursor::PreviousCell);
    QTextCursor cur = textCursor();
    cur.clearSelection();
    cur.setKeepPositionOnInsert(true);
    insertHtml(msgAction->getHtml());
    msgAction->setTextCursor(cur);

    messages.append(msgAction);
}
