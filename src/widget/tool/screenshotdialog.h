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

#ifndef SCREENSHODIALOG_H
#define SCREENSHODIALOG_H

#include <QDialog>

class QRubberBand;

class ScreenshotDialog : public QDialog
{
    Q_OBJECT
public:
    ScreenshotDialog(QRect &region);
    ~ScreenshotDialog();

protected:
    void mousePressEvent(QMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QMouseEvent *mouseEvent) override;

private:
    void calculateRect(QMouseEvent *mouseEvent, QRect &rect);
    QRect &region;
    QRubberBand *rubberBand;
    QPoint point;
    enum Status
    {
        Dead,
        Check, // To check whether mouse moved far enough.
        Alive
    };
    Status status;
};

#endif // SCREENSHODIALOG_H

