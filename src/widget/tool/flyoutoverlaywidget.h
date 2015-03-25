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

#ifndef FLYOUTOVERLAYWIDGET_HPP
#define FLYOUTOVERLAYWIDGET_HPP

#include <QWidget>

class QPropertyAnimation;

class FlyoutOverlayWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal flyoutPercent READ flyoutPercent WRITE setFlyoutPercent)
public:
    explicit FlyoutOverlayWidget(QWidget *parent = 0);
    ~FlyoutOverlayWidget();
    
    int animationDuration() const;
    void setAnimationDuration(int timeMs);
    
    qreal flyoutPercent() const;
    void setFlyoutPercent(qreal progress);
    
    bool isShown() const;
    
    void animateShow();
    void animateHide();
    
signals:
    
    void hidden();
    
private:
    
    void finishedAnimation();
    
    QWidget *container;
    QPropertyAnimation *animation;
    qreal percent = 1.0f;
    QPoint startPos;
    
};

#endif // FLYOUTOVERLAYWIDGET_HPP
