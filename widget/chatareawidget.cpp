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
#include "widget/tool/chatactions/chataction.h"
#include <QScrollBar>
#include <QDesktopServices>
#include <QTextTable>
#include <QAbstractTextDocumentLayout>
#include <QCoreApplication>
#include <QDebug>

ChatAreaWidget::ChatAreaWidget(QWidget *parent)
    : QTextBrowser(parent)
    , tableFrmt(nullptr)
    , nameWidth(75)
{
    setReadOnly(true);
    viewport()->setCursor(Qt::ArrowCursor);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setUndoRedoEnabled(false);

    setOpenExternalLinks(false);
    setOpenLinks(false);
    setAcceptRichText(false);
    setFrameStyle(QFrame::NoFrame);

    nameFormat.setAlignment(Qt::AlignRight);
    nameFormat.setNonBreakableLines(true);
    dateFormat.setAlignment(Qt::AlignLeft);
    dateFormat.setNonBreakableLines(true);

    connect(this, &ChatAreaWidget::anchorClicked, this, &ChatAreaWidget::onAnchorClicked);
    connect(verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(onSliderRangeChanged()));
}

ChatAreaWidget::~ChatAreaWidget()
{
    for (ChatAction* action : messages)
        delete action;
    messages.clear();

    if (tableFrmt)
        delete tableFrmt;
}

void ChatAreaWidget::mouseReleaseEvent(QMouseEvent * event)
{
    QTextEdit::mouseReleaseEvent(event);
    QPointF documentHitPost(event->pos().x() + horizontalScrollBar()->value(), event->pos().y() + verticalScrollBar()->value());
    int pos = this->document()->documentLayout()->hitTest(documentHitPost, Qt::ExactHit);
    if (pos > 0)
    {
        QTextCursor cursor(document());
        cursor.setPosition(pos);

        if(!cursor.atEnd())
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
                    qDebug() << "ChatAreaWidget::mouseReleaseEvent:" << widgetID << widgetBtn;
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

    checkSlider();

    QTextTable *chatTextTable = getMsgTable();
    QTextCursor cur = chatTextTable->cellAt(0, 2).firstCursorPosition();
    cur.clearSelection();
    cur.setKeepPositionOnInsert(true);
    chatTextTable->cellAt(0, 0).firstCursorPosition().setBlockFormat(nameFormat);
    chatTextTable->cellAt(0, 0).firstCursorPosition().insertHtml(msgAction->getName());
    chatTextTable->cellAt(0, 2).firstCursorPosition().insertHtml(msgAction->getMessage());
    chatTextTable->cellAt(0, 4).firstCursorPosition().setBlockFormat(dateFormat);
    chatTextTable->cellAt(0, 4).firstCursorPosition().insertHtml(msgAction->getDate());

    msgAction->setup(cur, this);

    messages.append(msgAction);
}

void ChatAreaWidget::onSliderRangeChanged()
{
    QScrollBar* scroll = verticalScrollBar();
    if (lockSliderToBottom)
        scroll->setValue(scroll->maximum());
}

void ChatAreaWidget::checkSlider()
{
    QScrollBar* scroll = verticalScrollBar();
    lockSliderToBottom = scroll && scroll->value() == scroll->maximum();
}

QTextTable *ChatAreaWidget::getMsgTable()
{
    if (tableFrmt == nullptr)
    {
        tableFrmt = new QTextTableFormat();
        tableFrmt->setCellSpacing(2);
        tableFrmt->setBorderStyle(QTextFrameFormat::BorderStyle_None);
        tableFrmt->setColumnWidthConstraints({QTextLength(QTextLength::FixedLength,nameWidth),
                                              QTextLength(QTextLength::FixedLength,2),
                                              QTextLength(QTextLength::PercentageLength,100),
                                              QTextLength(QTextLength::FixedLength,2),
                                              QTextLength(QTextLength::VariableLength,0)});
    }

    QTextCursor tc = textCursor();
    tc.movePosition(QTextCursor::End);
    QTextTable *chatTextTable = tc.insertTable(1, 5, *tableFrmt);

    return chatTextTable;
}

void ChatAreaWidget::setNameColWidth(int w)
{
    if (tableFrmt != nullptr)
    {
        delete tableFrmt;
        tableFrmt = nullptr;
    }

    nameWidth = w;
}
