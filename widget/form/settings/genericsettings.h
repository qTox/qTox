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

#include <QObject>
#include <QVBoxLayout>
#include "widget/form/settingswidget.h"

class GenericForm : public QObject
{
    Q_OBJECT
public:
    virtual void show(SettingsWidget& sw)
    {
        sw.body->layout()->addWidget(&body);
        body.show();
        sw.head->layout()->addWidget(&head);
        head.show();
    }

protected:
    QVBoxLayout layout;
    QHBoxLayout headLayout;
    QIcon icon;
    QLabel label;
    QWidget head, body;
    void prep() // call in subclass constructor
    {
        head.setLayout(&headLayout);
        headLayout.addWidget(&icon);
        headLayout.addWidget(&label);
        body.setLayout(&layout);
    }

};

#endif
