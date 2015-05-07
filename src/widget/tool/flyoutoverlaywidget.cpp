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

#include "flyoutoverlaywidget.h"

#include <QPropertyAnimation>
#include <QHBoxLayout>
#include <QPainter>
#include <QBitmap>
#include <QTimer>

FlyoutOverlayWidget::FlyoutOverlayWidget(QWidget *parent)
    : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);
    
    animation = new QPropertyAnimation(this, QByteArrayLiteral("flyoutPercent"), this);
    animation->setKeyValueAt(0, 0.0f);
    animation->setKeyValueAt(1, 1.0f);
    animation->setDuration(200);
    
    connect(animation, &QAbstractAnimation::finished, this, &FlyoutOverlayWidget::finishedAnimation);
    setFlyoutPercent(0);
    hide();
    
}

FlyoutOverlayWidget::~FlyoutOverlayWidget()
{
    
}

int FlyoutOverlayWidget::animationDuration() const
{
    return animation->duration();
}

void FlyoutOverlayWidget::setAnimationDuration(int timeMs)
{
    animation->setDuration(timeMs);
}

qreal FlyoutOverlayWidget::flyoutPercent() const
{
    return percent;
}

void FlyoutOverlayWidget::setFlyoutPercent(qreal progress)
{
    percent = progress;
    
    QSize self = size();
    setMask(QRegion(0, 0, self.width() * progress + 1, self.height()));
    move(startPos.x() + self.width() - self.width() * percent, startPos.y());
    setVisible (progress != 0);
    
}

bool FlyoutOverlayWidget::isShown() const
{
    return (percent == 1);
}

bool FlyoutOverlayWidget::isBeingAnimated() const
{
    return (animation->state() == QAbstractAnimation::Running);
}

bool FlyoutOverlayWidget::isBeingShown() const
{
    return (isBeingAnimated() && animation->direction() == QAbstractAnimation::Forward);
}

void FlyoutOverlayWidget::animateShow()
{
    if (percent == 1.0f)
        return;
    
    if (animation->state() != QAbstractAnimation::Running)
        this->startPos = pos();
    
    startAnimation(true);
}

void FlyoutOverlayWidget::animateHide()
{
    if (animation->state() != QAbstractAnimation::Running)
        this->startPos = pos();
    
    startAnimation(false);
}

void FlyoutOverlayWidget::finishedAnimation()
{
    bool hide = (animation->direction() == QAbstractAnimation::Backward);
    
    // Delay it by a few frames to let the system catch up on rendering
    if (hide)
        QTimer::singleShot(50, this, &FlyoutOverlayWidget::hidden);
    
}

void FlyoutOverlayWidget::startAnimation(bool forward)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, !forward);
    animation->setDirection(forward ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    animation->start();
    animation->setCurrentTime(animation->duration() * percent);
    
}
