/*
    Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
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

#include <QWidget>

class QPropertyAnimation;

class FlyoutOverlayWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal flyoutPercent READ flyoutPercent WRITE setFlyoutPercent)
public:
    explicit FlyoutOverlayWidget(QWidget* parent = nullptr);
    ~FlyoutOverlayWidget();

    int animationDuration() const;
    void setAnimationDuration(int timeMs);

    qreal flyoutPercent() const;
    void setFlyoutPercent(qreal progress);

    bool isShown() const;
    bool isBeingAnimated() const;
    bool isBeingShown() const;

    void animateShow();
    void animateHide();

signals:

    void hidden();

private:
    void finishedAnimation();
    void startAnimation(bool forward);

    QWidget* container;
    QPropertyAnimation* animation;
    qreal percent = 1.0;
    QPoint startPos;
};
