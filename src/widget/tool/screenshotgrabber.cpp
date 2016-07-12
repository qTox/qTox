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

#include "screenshotgrabber.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QScreen>
#include <QTimer>

#include "screengrabberchooserrectitem.h"
#include "screengrabberoverlayitem.h"
#include "toolboxgraphicsitem.h"
#include "src/widget/widget.h"

ScreenshotGrabber::ScreenshotGrabber(QObject* parent)
    : QObject(parent)
    , mKeysBlocked(false)
    , scene(0)
    , mQToxVisible(true)
{
    window = new QGraphicsView (scene); // Top-level widget
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowFlags(Qt::FramelessWindowHint | Qt::BypassWindowManagerHint);
    window->setContentsMargins(0, 0, 0, 0);
    window->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    window->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    window->setFrameShape(QFrame::NoFrame);
    window->installEventFilter(this);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    pixRatio = QApplication::primaryScreen()->devicePixelRatio();
#endif

    // Scale window down by devicePixelRatio to show full screen region
    window->scale(1 / pixRatio, 1 / pixRatio);

    setupScene();
}

void ScreenshotGrabber::reInit()
{
    window->resetCachedContent();
    setupScene();
    showGrabber();
    mKeysBlocked = false;
}

ScreenshotGrabber::~ScreenshotGrabber()
{
    delete scene;
}

bool ScreenshotGrabber::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
        return handleKeyPress(static_cast<QKeyEvent*>(event));

    return QObject::eventFilter(object, event);
}

void ScreenshotGrabber::showGrabber()
{
    this->screenGrab = grabScreen();
    this->screenGrabDisplay->setPixmap(this->screenGrab);
    this->window->show();
    this->window->setFocus();
    this->window->grabKeyboard();

    QRect fullGrabbedRect = screenGrab.rect();
    QRect rec = QApplication::primaryScreen()->virtualGeometry();

    this->window->setGeometry(rec);
    this->scene->setSceneRect(fullGrabbedRect);
    this->overlay->setRect(fullGrabbedRect);

    adjustTooltipPosition();
}

bool ScreenshotGrabber::handleKeyPress(QKeyEvent* event)
{
    if (mKeysBlocked)
        return false;

    if (event->key() == Qt::Key_Escape)
        reject();
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        acceptRegion();
    else if (event->key() == Qt::Key_Space) {
        mKeysBlocked = true;

        if (mQToxVisible)
            hideVisibleWindows();
        else
            restoreHiddenWindows();

        window->hide();
        QTimer::singleShot(350, this, SLOT(reInit()));
    }
    else
        return false;

    return true;
}

void ScreenshotGrabber::acceptRegion()
{
    QRect rect = this->chooserRect->chosenRect();
    if (rect.width() < 1 || rect.height() < 1)
        return;

    emit regionChosen(rect);
    qDebug() << "Screenshot accepted, chosen region" << rect;
    QPixmap pixmap = this->screenGrab.copy(rect);
    this->window->close();
    restoreHiddenWindows();
    emit screenshotTaken(pixmap);
}

void ScreenshotGrabber::setupScene()
{
    delete scene;
    scene = new QGraphicsScene;
    window->setScene(scene);

    this->overlay = new ScreenGrabberOverlayItem(this);
    this->helperToolbox = new ToolBoxGraphicsItem;

    this->screenGrabDisplay = scene->addPixmap(this->screenGrab);
    this->helperTooltip = scene->addText(QString());

    // Scale UI elements up by devicePixelRatio to compensate for window downscaling
    this->helperToolbox->setScale(pixRatio);
    this->helperTooltip->setScale(pixRatio);

    scene->addItem(this->overlay);
    this->chooserRect = new ScreenGrabberChooserRectItem(scene);
    scene->addItem(this->helperToolbox);

    this->helperToolbox->addToGroup(this->helperTooltip);
    this->helperTooltip->setDefaultTextColor(Qt::black);
    useNothingSelectedTooltip();

    connect(this->chooserRect, &ScreenGrabberChooserRectItem::doubleClicked, this, &ScreenshotGrabber::acceptRegion);
    connect(this->chooserRect, &ScreenGrabberChooserRectItem::regionChosen, this, &ScreenshotGrabber::chooseHelperTooltipText);
    connect(this->chooserRect, &ScreenGrabberChooserRectItem::regionChosen, this->overlay, &ScreenGrabberOverlayItem::setChosenRect);
}

void ScreenshotGrabber::useNothingSelectedTooltip()
{
    helperTooltip->setHtml(tr("Click and drag to select a region. Press <b>Space</b> to hide/show qTox window, or <b>Escape</b> to cancel.",
                              "Help text shown when no region has been selected yet"));
    adjustTooltipPosition();
}

void ScreenshotGrabber::useRegionSelectedTooltip()
{
    helperTooltip->setHtml(tr("Press <b>Enter</b> to send a screenshot of the selection, <b>Space</b> to hide/show qTox window, or <b>Escape</b> to cancel.",
                              "Help text shown when a region has been selected"));
    adjustTooltipPosition();
}

void ScreenshotGrabber::chooseHelperTooltipText(QRect rect)
{
    if (rect.size().isNull())
        useNothingSelectedTooltip();
    else
        useRegionSelectedTooltip();
}

/**
 * @internal
 *
 * Align the tooltip centred at top of screen with the mouse cursor.
 */
void ScreenshotGrabber::adjustTooltipPosition()
{
    QRect recGL = QGuiApplication::primaryScreen()->virtualGeometry();
    QRect rec = qApp->desktop()->screenGeometry(QCursor::pos());
    const QRectF ttRect = this->helperToolbox->childrenBoundingRect();
    int x = abs(recGL.x()) + rec.x() + ((rec.width() - ttRect.width()) / 2);
    int y = abs(recGL.y()) + rec.y();

    // Multiply by devicePixelRatio to get centered positions under scaling
    helperToolbox->setX(x * pixRatio);
    helperToolbox->setY(y * pixRatio);
}

void ScreenshotGrabber::reject()
{
    qDebug() << "Rejected screenshot";
    this->window->close();
    restoreHiddenWindows();
}

QPixmap ScreenshotGrabber::grabScreen()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect rec = screen->virtualGeometry();

    // Multiply by devicePixelRatio to get actual desktop size
    return screen->grabWindow(QApplication::desktop()->winId(),
                              rec.x() * pixRatio,
                              rec.y() * pixRatio,
                              rec.width() * pixRatio,
                              rec.height() * pixRatio);
}

void ScreenshotGrabber::hideVisibleWindows()
{
    foreach(QWidget* w, qApp->topLevelWidgets()) {
        if (w != window && w->isVisible()) {
            mHiddenWindows << w;
            w->setVisible(false);
        }
    }

    mQToxVisible = false;
}

void ScreenshotGrabber::restoreHiddenWindows()
{
    foreach(QWidget* w, mHiddenWindows) {
        if (w)
            w->setVisible(true);
    }

    mHiddenWindows.clear();
    mQToxVisible = true;
}

void ScreenshotGrabber::beginRectChooser(QGraphicsSceneMouseEvent* event)
{
    QPointF pos = event->scenePos();
    this->chooserRect->setX(pos.x());
    this->chooserRect->setY(pos.y());
    this->chooserRect->beginResize(event->scenePos());
}
