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

#ifndef EDITABLELABELWIDGET_HPP
#define EDITABLELABELWIDGET_HPP

#include "copyableelidelabel.h"
#include "esclineedit.h"

#include <QLineEdit>
#include <QStackedWidget>

class ClickableCopyableElideLabel : public CopyableElideLabel
{
    Q_OBJECT
public:
    explicit ClickableCopyableElideLabel(QWidget* parent = 0);

protected:
    bool event(QEvent* event) Q_DECL_OVERRIDE;

signals:
    void clicked();

};

class EditableLabelWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit EditableLabelWidget(QWidget* parent = 0);

    ClickableCopyableElideLabel* label;
    EscLineEdit* lineEdit;

    void setText(const QString& text);
    QString text();

private:
    static const int LINE_SPACING_OFFSET = 2;
    bool isSubmitting;

private slots:
    void onLabelChangeSubmited();
    void onLabelChangeCancelled();
    void onLabelClicked();

signals:
    void textChanged(QString newText, QString oldText);
    
};

#endif // EDITABLELABELWIDGET_HPP
