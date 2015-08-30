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

#ifndef SCREENSHOTGRABBER_H
#define SCREENSHOTGRABBER_H

#include <QWidget>
#include <QPixmap>
#include <QPoint>
#include <QTimer>

class ScreenGrabberChooserRectItem;
class QGraphicsSceneMouseEvent;
class ScreenGrabberOverlayItem;
class QGraphicsPixmapItem;
class ToolBoxGraphicsItem;
class QGraphicsRectItem;
class QGraphicsTextItem;
class QGraphicsScene;
class QGraphicsView;

class ScreenshotGrabber : public QWidget
{
    Q_OBJECT
public:

    ScreenshotGrabber(QWidget* parent);
    ~ScreenshotGrabber() override;

    bool eventFilter(QObject* object, QEvent* event);

public slots:

    void showGrabber();
    void acceptRegion();
    void reInit();

signals:
    void screenshotTaken(const QPixmap &pixmap);
    void rejected();

private:
    friend class ScreenGrabberOverlayItem;
    // for exception multiple handling during switching window
    bool blocked = false;

    void setupWindow();
    void setupScene(QGraphicsScene* scene);

    void useNothingSelectedTooltip();
    void useRegionSelectedTooltip();
    void chooseHelperTooltipText(QRect rect);
    void adjustTooltipPosition();

    bool handleKeyPress(QKeyEvent* event);
    void reject();

    QRect getSystemScreenRect();
    void adjustWindowSize();
    QPixmap grabScreen();

    void beginRectChooser(QGraphicsSceneMouseEvent* event);

    QPixmap screenGrab;
    QGraphicsScene* scene;
    QGraphicsView* window;
    QGraphicsPixmapItem* screenGrabDisplay;
    ScreenGrabberOverlayItem* overlay;
    ScreenGrabberChooserRectItem* chooserRect;
    ToolBoxGraphicsItem* helperToolbox;
    QGraphicsTextItem* helperTooltip;
};


#endif // SCREENSHOTGRABBER_H

