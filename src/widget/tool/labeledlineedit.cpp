/*
    Copyright © 2015 by The qTox Project

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

#include "labeledlineedit.h"
#include <QLineEdit>
#include <QLabel>
#include <QStyle>

LabeledLineEdit::LabeledLineEdit(QWidget* parent)
    : QLineEdit(parent)
{
    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->show();

    const int labelMargin = 4;
    label->setContentsMargins(labelMargin, 0, labelMargin, 0);
}

void LabeledLineEdit::setLabelText(const QString& text)
{
    label->setText(text);
    setLabelGeometry();
}

void LabeledLineEdit::resizeEvent(QResizeEvent* event)
{
    setLabelGeometry();
    QWidget::resizeEvent(event);
}

void LabeledLineEdit::moveEvent(QMoveEvent *event)
{
    setLabelGeometry();
    QWidget::moveEvent(event);
}

void LabeledLineEdit::setLabelGeometry()
{
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth) + 1;

    int labelWidth = label->sizeHint().width() + frameWidth;
    label->resize(labelWidth, height() - frameWidth * 2);
    label->move(width() - labelWidth - frameWidth, frameWidth);
    setTextMargins(0, 0, labelWidth, 0);
}
