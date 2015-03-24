/*
    Copyright (C) 2015 by Project Tox <https://tox.im>

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

#include "screenshotgrabber.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QDesktopWidget>
#include <QGraphicsView>
#include <QApplication>
#include <QMouseEvent>
#include <QScreen>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

#include "screengrabberchooserrectitem.hpp"
#include "screengrabberoverlayitem.hpp"

ScreenshotGrabber::ScreenshotGrabber(QWidget* parent)
    : QWidget(parent)
{
    
    QGraphicsScene *scene = new QGraphicsScene;
    
    this->window = new QGraphicsView (scene); // Top-level widget
    
    this->window->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    this->window->setAttribute(Qt::WA_DeleteOnClose);
    this->window->setContentsMargins(0, 0, 0, 0);
    this->window->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->window->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->window->setFrameShape(QFrame::NoFrame);
    
    
    this->overlay = new ScreenGrabberOverlayItem(this);
    this->chooserRect = new ScreenGrabberChooserRectItem;
    
    this->screenGrabDisplay = scene->addPixmap(this->screenGrab);
    scene->addItem(this->overlay);
    scene->addItem(this->chooserRect);
    
    this->window->installEventFilter(this);
    installEventFilter(this);
    
    connect(this->window, &QObject::destroyed, this, &QObject::deleteLater);
    connect(this->chooserRect, &ScreenGrabberChooserRectItem::doubleClicked, this, &ScreenshotGrabber::acceptRegion);
}

ScreenshotGrabber::~ScreenshotGrabber()
{
}

bool ScreenshotGrabber::eventFilter(QObject *object, QEvent *event)
{
    if (event->type () == QEvent::KeyPress) {
        return handleKeyPress(static_cast<QKeyEvent *>(event));
    }
    
    return QWidget::eventFilter(object, event);
}

void ScreenshotGrabber::showGrabber()
{
    this->screenGrab = grabScreen();
    this->screenGrabDisplay->setPixmap(this->screenGrab);
    this->window->show();
    this->window->setFocus();
    adjustWindowSize();
}

bool ScreenshotGrabber::handleKeyPress(QKeyEvent *event)
{
    if (event->key () == Qt::Key_Escape) {
        reject();
    } else if (event->key () == Qt::Key_Return || event->key () == Qt::Key_Enter) {
        acceptRegion();
    } else {
        return false;
    }
    
    return true;
}

void ScreenshotGrabber::acceptRegion()
{
    QRect rect = this->chooserRect->chosenRect();
    if (rect.width () < 1 || rect.height() < 1) {
        return;
    }
    
    // 
    qDebug() << "Screenshot accepted, chosen region" << rect;
    emit screenshotTaken(this->screenGrab.copy(rect));
    this->window->close();
}

void ScreenshotGrabber::reject()
{
    qDebug() << "Rejected screenshot";
    this->window->close();
    
}

QRect ScreenshotGrabber::getSystemScreenRect()
{
    return QApplication::primaryScreen()->virtualGeometry();
}

void ScreenshotGrabber::adjustWindowSize()
{
    QRect systemScreenRect = getSystemScreenRect();
    qDebug() << "adjusting grabber size to" << systemScreenRect;
    
    this->window->setGeometry(systemScreenRect);
    this->window->scene()->setSceneRect(systemScreenRect);
    this->overlay->setRect(systemScreenRect);
}

QPixmap ScreenshotGrabber::grabScreen() {
    return QApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());
}

void ScreenshotGrabber::beginRectChooser(QGraphicsSceneMouseEvent *event)
{
    QPointF pos = event->scenePos();
    this->chooserRect->setX(pos.x());
    this->chooserRect->setY(pos.y());
    this->chooserRect->beginResize();
}
