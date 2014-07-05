/*
    Copyright (C) 2013 by Maxim Biro <nurupo.contributions@gmail.com>
    
    This file is part of Tox Qt GUI.
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    
    See the COPYING file for more details.
*/

#include "editablelabelwidget.h"

#include <QApplication>
#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QVBoxLayout>

ClickableCopyableElideLabel::ClickableCopyableElideLabel(QWidget* parent) :
    CopyableElideLabel(parent)
{
}

bool ClickableCopyableElideLabel::event(QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            emit clicked();
        }
    } else if (event->type() == QEvent::Enter) {
        QApplication::setOverrideCursor(QCursor(Qt::PointingHandCursor));
    } else if (event->type() == QEvent::Leave) {
        QApplication::restoreOverrideCursor();
    }

    return CopyableElideLabel::event(event);
}

EditableLabelWidget::EditableLabelWidget(QWidget* parent) :
    QStackedWidget(parent), isSubmitting(false)
{
    label = new ClickableCopyableElideLabel(this);

    connect(label, &ClickableCopyableElideLabel::clicked,   this, &EditableLabelWidget::onLabelClicked);

    lineEdit = new EscLineEdit(this);
    lineEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    lineEdit->setMinimumHeight(label->fontMetrics().lineSpacing() + LINE_SPACING_OFFSET);

    // Set dark background for >windows
    //QColor toxDarkAsMySoul(28,28,28);
    //QPalette darkPal;
    //darkPal.setColor(QPalette::Window, toxDarkAsMySoul);
    //darkPal.setColor(QPalette::Base, toxDarkAsMySoul);
    //lineEdit->setPalette(darkPal);

    connect(lineEdit, &EscLineEdit::editingFinished,        this, &EditableLabelWidget::onLabelChangeSubmited);
    connect(lineEdit, &EscLineEdit::escPressed,             this, &EditableLabelWidget::onLabelChangeCancelled);

    addWidget(label);
    addWidget(lineEdit);

    setCurrentWidget(label);
}

void EditableLabelWidget::setText(const QString& text)
{
    label->setText(text);
    lineEdit->setText(text);
}

QString EditableLabelWidget::text()
{
    return label->text();
}

void EditableLabelWidget::onLabelChangeSubmited()
{
    if (isSubmitting) {
        return;
    }
    isSubmitting = true;

    QString oldText = label->text();
    QString newText = lineEdit->text();
    // `lineEdit->clearFocus()` triggers `onLabelChangeSubmited()`, we use `isSubmitting` as a workaround
    lineEdit->clearFocus();
    setCurrentWidget(label);

    if (oldText != newText) {
        label->setText(newText);
        emit textChanged(newText, oldText);
    }

    isSubmitting = false;
}

void EditableLabelWidget::onLabelChangeCancelled()
{
    // order of calls matters, since clearFocus() triggers EditableLabelWidget::onLabelChangeSubmited()
    lineEdit->setText(label->text());
    lineEdit->clearFocus();
    setCurrentWidget(label);
}

void EditableLabelWidget::onLabelClicked()
{
    setCurrentWidget(lineEdit);
    lineEdit->setFocus();
}
