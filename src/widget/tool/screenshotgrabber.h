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

#include <QPixmap>
#include <QPointer>

class QGraphicsSceneMouseEvent;
class QGraphicsPixmapItem;
class QGraphicsRectItem;
class QGraphicsTextItem;
class QGraphicsScene;
class QGraphicsView;
class QKeyEvent;
class ScreenGrabberChooserRectItem;
class ScreenGrabberOverlayItem;
class ToolBoxGraphicsItem;

class ScreenshotGrabber : public QObject
{
    Q_OBJECT
public:
    ScreenshotGrabber();
    ~ScreenshotGrabber() override;

    bool eventFilter(QObject* object, QEvent* event) override;

    void showGrabber();

public slots:
    void acceptRegion();
    void reInit();

signals:
    void screenshotTaken(const QPixmap& pixmap);
    void regionChosen(QRect region);
    void rejected();

private:
    friend class ScreenGrabberOverlayItem;
    bool mKeysBlocked;

    void setupScene();

    void useNothingSelectedTooltip();
    void useRegionSelectedTooltip();
    void chooseHelperTooltipText(QRect rect);
    void adjustTooltipPosition();

    bool handleKeyPress(QKeyEvent* event);
    void reject();

    QPixmap grabScreen();

    void hideVisibleWindows();
    void restoreHiddenWindows();

    void beginRectChooser(QGraphicsSceneMouseEvent* event);

private:
    QPixmap screenGrab;
    QGraphicsScene* scene;
    QGraphicsView* window;
    QGraphicsPixmapItem* screenGrabDisplay;
    ScreenGrabberOverlayItem* overlay;
    ScreenGrabberChooserRectItem* chooserRect;
    ToolBoxGraphicsItem* helperToolbox;
    QGraphicsTextItem* helperTooltip;

    qreal pixRatio = 1.0;

    bool mQToxVisible;
    QVector<QPointer<QWidget>> mHiddenWindows;
};
