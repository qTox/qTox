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

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QDesktopWidget>
#include <QGraphicsView>
#include <QApplication>
#include <QMouseEvent>
#include <QScreen>
#include <QDebug>

#include "screengrabberchooserrectitem.h"
#include "screengrabberoverlayitem.h"
#include "toolboxgraphicsitem.h"

ScreenshotGrabber::ScreenshotGrabber(QWidget* parent)
    : QWidget(parent)
{
    
    QGraphicsScene* scene = new QGraphicsScene;
    this->window = new QGraphicsView (scene); // Top-level widget
    setupWindow();
    setupScene(scene);
    
    installEventFilter(this);
    
}

ScreenshotGrabber::~ScreenshotGrabber()
{
}

bool ScreenshotGrabber::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
        return handleKeyPress(static_cast<QKeyEvent*>(event));
    
    return QWidget::eventFilter(object, event);
}

void ScreenshotGrabber::showGrabber()
{
    this->screenGrab = grabScreen();
    this->screenGrabDisplay->setPixmap(this->screenGrab);
    this->window->show();
    this->window->setFocus();
    this->window->grabKeyboard();
    adjustWindowSize();
    adjustTooltipPosition();
}

bool ScreenshotGrabber::handleKeyPress(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        reject();
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        acceptRegion();
    else
        return false;
    
    return true;
}

void ScreenshotGrabber::acceptRegion()
{
    QRect rect = this->chooserRect->chosenRect();
    if (rect.width() < 1 || rect.height() < 1)
        return;
    
    // 
    qDebug() << "Screenshot accepted, chosen region" << rect;
    emit screenshotTaken(this->screenGrab.copy(rect));
    this->window->close();
}

void ScreenshotGrabber::setupWindow()
{
    this->window->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    this->window->setAttribute(Qt::WA_DeleteOnClose);
    this->window->setContentsMargins(0, 0, 0, 0);
    this->window->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->window->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->window->setFrameShape(QFrame::NoFrame);
    
    connect(this->window, &QObject::destroyed, this, &QObject::deleteLater);
    this->window->installEventFilter(this);
}

void ScreenshotGrabber::setupScene(QGraphicsScene* scene)
{
    this->overlay = new ScreenGrabberOverlayItem(this);
    this->helperToolbox = new ToolBoxGraphicsItem;
    
    this->screenGrabDisplay = scene->addPixmap(this->screenGrab);
    this->helperTooltip = scene->addText(QString());
    scene->addItem(this->overlay);
    this->chooserRect = new ScreenGrabberChooserRectItem(scene);
    scene->addItem(this->helperToolbox);
    
    this->helperToolbox->addToGroup(this->helperTooltip);
    this->helperTooltip->setDefaultTextColor(Qt::black);
    useNothingSelectedTooltip();
    
    connect(this->chooserRect, &ScreenGrabberChooserRectItem::doubleClicked, this, &ScreenshotGrabber::acceptRegion);
    connect(this->chooserRect, &ScreenGrabberChooserRectItem::regionChosen, this, &ScreenshotGrabber::useRegionSelectedTooltip);
    connect(this->chooserRect, &ScreenGrabberChooserRectItem::regionChosen, this->overlay, &ScreenGrabberOverlayItem::setChosenRect);
}

void ScreenshotGrabber::useNothingSelectedTooltip()
{
    this->helperTooltip->setHtml(tr("Click and drag to select a region. Press <b>Escape</b> to cancel.",
                                    "Help text shown when no region has been selected yet"));
    adjustTooltipPosition();
}

void ScreenshotGrabber::useRegionSelectedTooltip()
{
    this->helperTooltip->setHtml(tr("Press <b>Enter</b> to send a screenshot of the selected region or select a new region. Press <b>Escape</b> to cancel.",
                                    "Help text shown when a region has been selected"));
    adjustTooltipPosition();
}

void ScreenshotGrabber::adjustTooltipPosition()
{
    QRectF size = this->helperToolbox->childrenBoundingRect();
    QRect screenRect = QApplication::desktop()->screen()->rect();
    
    // Align the toolbox center-top.
    this->helperToolbox->setX(screenRect.x() + (screenRect.width() - size.width() + size.x()) / 2);
    this->helperToolbox->setY(screenRect.y());
    
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

void ScreenshotGrabber::beginRectChooser(QGraphicsSceneMouseEvent* event)
{
    QPointF pos = event->scenePos();
    this->chooserRect->setX(pos.x());
    this->chooserRect->setY(pos.y());
    this->chooserRect->beginResize(event->scenePos());
}
