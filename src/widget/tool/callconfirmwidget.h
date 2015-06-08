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


#ifndef CALLCONFIRMWIDGET_H
#define CALLCONFIRMWIDGET_H

#include <QWidget>
#include <QRect>
#include <QPolygon>
#include <QBrush>

class QPaintEvent;
class QShowEvent;
class Friend;

/// This is a widget with dialog buttons to accept/reject a call
/// It tracks the position of another widget called the anchor
/// and looks like a bubble at the bottom of that widget.
class CallConfirmWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit CallConfirmWidget(const QWidget *Anchor, const Friend& f);

signals:
    void accepted();
    void rejected();

protected:
    virtual void paintEvent(QPaintEvent* event) final override;
    virtual void showEvent(QShowEvent* event) final override;

protected slots:
    void reposition(); ///< Recalculate our positions to track the anchor

private:
    const QWidget* anchor; ///< The widget we're going to be tracking
    const Friend& f; ///< The friend on whose chat form we should appear

    QRect mainRect;
    QPolygon spikePoly;
    QBrush brush;

    const int rectW, rectH;
    const int spikeW, spikeH;
    const int roundedFactor; ///< By how much are the corners of the main rect rounded
    const qreal rectRatio; ///< Used to correct the rounding factors on non-square rects
};

#endif // CALLCONFIRMWIDGET_H
