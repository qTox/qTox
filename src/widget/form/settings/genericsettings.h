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

#ifndef GENERICFORM_H
#define GENERICFORM_H

#include <QWidget>
#include "src/widget/form/settingswidget.h"

class GenericForm : public QWidget
{
    Q_OBJECT
public:
    GenericForm(const QString &name, const QPixmap &icon) : formName(name), formIcon(icon) {;}
    ~GenericForm() {;}

    virtual void present() {}
    QString getFormName() {return formName;}
    QPixmap getFormIcon() {return formIcon;}

protected:
    QString formName;
    QPixmap formIcon;
};

#endif
