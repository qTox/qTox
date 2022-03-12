/*
    Copyright © 2015-2019 by The qTox Project Contributors

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


#pragma once

#include <QBrush>
#include <QPolygon>
#include <QRect>
#include <QWidget>

class QPaintEvent;
class QShowEvent;
class Settings;
class Style;

class CallConfirmWidget final : public QWidget
{
    Q_OBJECT
public:
    CallConfirmWidget(Settings& settings, Style& style, const QWidget* anchor_);

signals:
    void accepted();
    void rejected();

public slots:
    void reposition();

protected:
    void paintEvent(QPaintEvent* event) final;
    void showEvent(QShowEvent* event) final;
    void hideEvent(QHideEvent* event) final;
    bool eventFilter(QObject* object, QEvent* event) final;

private:
    const QWidget* anchor;

    QRect mainRect;
    QPolygon spikePoly;
    QBrush brush;

    const int rectW, rectH;
    const int spikeW, spikeH;
    const int roundedFactor;
    const qreal rectRatio;
};
