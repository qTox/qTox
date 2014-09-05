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

ChatAreaWidget::ChatAreaWidget(QWidget *parent) :
    QTextEdit(parent)
{
    setReadOnly(true);
    viewport()->setCursor(Qt::ArrowCursor);
    setContextMenuPolicy(Qt::CustomContextMenu);
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
                QString image = format.toImageFormat().name();
                if (QRegExp("^data:ftrans.*").exactMatch(image))
                {
                    image = image.right(image.length() - 12);
                    int endpos = image.indexOf("/png;base64");
                    image = image.left(endpos);
                    int middlepos = image.indexOf(".");
                    emit onFileTranfertInterract(image.left(middlepos),image.right(image.length() - middlepos - 1));
                }
            }
        }
    }
}
