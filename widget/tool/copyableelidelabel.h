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

#ifndef COPYABLEELIDELABEL_HPP
#define COPYABLEELIDELABEL_HPP

#include "elidelabel.h"

class CopyableElideLabel : public ElideLabel
{
    Q_OBJECT
public:
    explicit CopyableElideLabel(QWidget* parent = 0);

private:
    QAction* actionCopy;

private slots:
    void showContextMenu(const QPoint& pos);

};

#endif // COPYABLEELIDELABEL_HPP
