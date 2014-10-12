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

#ifndef MASKABLEPIXMAPWIDGET_H
#define MASKABLEPIXMAPWIDGET_H

#include <QWidget>

class MaskablePixmapWidget : public QWidget
{
    Q_OBJECT
public:
    MaskablePixmapWidget(QWidget *parent, QSize size, QString maskName = QString());

    void autopickBackground();
    void setBackground(QColor color);
    void setClickable(bool clickable);
    void setPixmap(const QPixmap &pmap, QColor background);
    void setPixmap(const QPixmap &pmap);
    QPixmap getPixmap() const;

signals:
    void clicked();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);

private:
    QPixmap pixmap;
    QPixmap mask;
    QPixmap renderTarget;
    QSize size;
    QString maskName;
    QColor backgroundColor;
    bool clickable;
};

#endif // MASKABLEPIXMAPWIDGET_H
