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

ScreenshotGrabber::ScreenshotGrabber()
    : QObject()
    , mKeysBlocked(false)
    , scene(nullptr)
    , mQToxVisible(true)
{
    window = new QGraphicsView(scene); // Top-level widget
    window->setWindowFlags(Qt::FramelessWindowHint | Qt::BypassWindowManagerHint);
    window->setContentsMargins(0, 0, 0, 0);
    window->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    window->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    window->setFrameShape(QFrame::NoFrame);
    window->installEventFilter(this);
    pixRatio = QApplication::primaryScreen()->devicePixelRatio();

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
    delete window;
}

bool ScreenshotGrabber::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
        return handleKeyPress(static_cast<QKeyEvent*>(event));

    return QObject::eventFilter(object, event);
}

void ScreenshotGrabber::showGrabber()
{
    screenGrab = grabScreen();
    screenGrabDisplay->setPixmap(screenGrab);
    window->show();
    window->setFocus();
    window->grabKeyboard();

    QRect fullGrabbedRect = screenGrab.rect();
    QRect rec = QApplication::primaryScreen()->virtualGeometry();

    window->setGeometry(rec);
    scene->setSceneRect(fullGrabbedRect);
    overlay->setRect(fullGrabbedRect);

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
    } else
        return false;

    return true;
}

void ScreenshotGrabber::acceptRegion()
{
    QRect rect = chooserRect->chosenRect();
    if (rect.width() < 1 || rect.height() < 1)
        return;

    // Scale the accepted region from DIPs to actual pixels
    rect.setRect(rect.x() * pixRatio, rect.y() * pixRatio, rect.width() * pixRatio,
                 rect.height() * pixRatio);

    emit regionChosen(rect);
    qDebug() << "Screenshot accepted, chosen region" << rect;
    QPixmap pixmap = screenGrab.copy(rect);
    restoreHiddenWindows();
    emit screenshotTaken(pixmap);

    deleteLater();
}

void ScreenshotGrabber::setupScene()
{
    delete scene;
    scene = new QGraphicsScene;
    window->setScene(scene);

    overlay = new ScreenGrabberOverlayItem(this);
    helperToolbox = new ToolBoxGraphicsItem;

    screenGrabDisplay = scene->addPixmap(screenGrab);
    helperTooltip = scene->addText(QString());

    scene->addItem(overlay);
    chooserRect = new ScreenGrabberChooserRectItem(scene);
    scene->addItem(helperToolbox);

    helperToolbox->addToGroup(helperTooltip);
    helperTooltip->setDefaultTextColor(Qt::black);
    useNothingSelectedTooltip();

    connect(chooserRect, &ScreenGrabberChooserRectItem::doubleClicked, this,
            &ScreenshotGrabber::acceptRegion);
    connect(chooserRect, &ScreenGrabberChooserRectItem::regionChosen, this,
            &ScreenshotGrabber::chooseHelperTooltipText);
    connect(chooserRect, &ScreenGrabberChooserRectItem::regionChosen, overlay,
            &ScreenGrabberOverlayItem::setChosenRect);
}

void ScreenshotGrabber::useNothingSelectedTooltip()
{
    helperTooltip->setHtml(
        tr("Click and drag to select a region. Press %1 to "
           "hide/show qTox window, or %2 to cancel.",
           "Help text shown when no region has been selected yet")
            .arg(QString("<b>%1</b>").arg(tr("Space", "[Space] key on the keyboard")),
                 QString("<b>%1</b>").arg(tr("Escape", "[Escape] key on the keyboard"))));
    adjustTooltipPosition();
}

void ScreenshotGrabber::useRegionSelectedTooltip()
{
    helperTooltip->setHtml(
        tr("Press %1 to send a screenshot of the selection, "
           "%2 to hide/show qTox window, or %3 to cancel.",
           "Help text shown when a region has been selected")
            .arg(QString("<b>%1</b>").arg(tr("Enter", "[Enter] key on the keyboard")),
                 QString("<b>%1</b>").arg(tr("Space", "[Space] key on the keyboard")),
                 QString("<b>%1</b>").arg(tr("Escape", "[Escape] key on the keyboard"))));
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
 * @brief Align the tooltip centered at top of screen with the mouse cursor.
 */
void ScreenshotGrabber::adjustTooltipPosition()
{
    QRect recGL = QGuiApplication::primaryScreen()->virtualGeometry();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    const auto rec = QGuiApplication::screenAt(QCursor::pos())->geometry();
#else
    const auto rec = qApp->desktop()->screenGeometry(QCursor::pos());
#endif
    const QRectF ttRect = helperToolbox->childrenBoundingRect();
    int x = qAbs(recGL.x()) + rec.x() + ((rec.width() - ttRect.width()) / 2);
    int y = qAbs(recGL.y()) + rec.y();

    helperToolbox->setX(x);
    helperToolbox->setY(y);
}

void ScreenshotGrabber::reject()
{
    restoreHiddenWindows();
    deleteLater();
}

QPixmap ScreenshotGrabber::grabScreen()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect rec = screen->virtualGeometry();

    // Multiply by devicePixelRatio to get actual desktop size
    return screen->grabWindow(QApplication::desktop()->winId(), rec.x() * pixRatio,
                              rec.y() * pixRatio, rec.width() * pixRatio, rec.height() * pixRatio);
}

void ScreenshotGrabber::hideVisibleWindows()
{
    foreach (QWidget* w, qApp->topLevelWidgets()) {
        if (w != window && w->isVisible()) {
            mHiddenWindows << w;
            w->setVisible(false);
        }
    }

    mQToxVisible = false;
}

void ScreenshotGrabber::restoreHiddenWindows()
{
    foreach (QWidget* w, mHiddenWindows) {
        if (w)
            w->setVisible(true);
    }

    mHiddenWindows.clear();
    mQToxVisible = true;
}

void ScreenshotGrabber::beginRectChooser(QGraphicsSceneMouseEvent* event)
{
    QPointF pos = event->scenePos();
    chooserRect->setX(pos.x());
    chooserRect->setY(pos.y());
    chooserRect->beginResize(event->scenePos());
}
