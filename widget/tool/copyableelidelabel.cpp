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

#include "copyableelidelabel.h"

#include <QApplication>
#include <QMenu>
#include <QClipboard>

CopyableElideLabel::CopyableElideLabel(QWidget* parent) :
    ElideLabel(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &CopyableElideLabel::customContextMenuRequested, this, &CopyableElideLabel::showContextMenu);

    actionCopy  = new QAction(CopyableElideLabel::tr("Copy"), this);
    connect(actionCopy, &QAction::triggered, [this]() {
        QApplication::clipboard()->setText(text());
    });
}

void CopyableElideLabel::showContextMenu(const QPoint& pos)
{
    if (text().length() == 0) {
        return;
    }

    QPoint globalPos = mapToGlobal(pos);

    QMenu contextMenu;
    contextMenu.addAction(actionCopy);

    contextMenu.exec(globalPos);
}
