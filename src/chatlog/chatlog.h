/*
    Copyright (C) 2014 by Project Tox <https://tox.im>

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

#ifndef CHATLOG_H
#define CHATLOG_H

#include <QGraphicsView>
#include <QDateTime>
#include <QMarginsF>

#include "chatline.h"
#include "chatmessage.h"

class QGraphicsScene;
class QGraphicsRectItem;
class QTimer;
class ChatLineContent;
class ToxFile;

class ChatLog : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ChatLog(QWidget* parent = 0);

    void insertChatlineAtBottom(ChatLine::Ptr l);
    void insertChatlineOnTop(ChatLine::Ptr l);
    void insertChatlineOnTop(const QList<ChatLine::Ptr>& newLines);
    void clearSelection();
    void clear();
    void copySelectedText() const;
    void setTypingNotification(ChatLine::Ptr notification);
    void setTypingNotificationVisible(bool visible);
    QString getSelectedText() const;
    QString toPlainText() const;

    bool isEmpty() const;
    bool hasTextToBeCopied() const;

    ChatLine::Ptr getTypingNotification() const;

protected:
    QRectF calculateSceneRect() const;
    QRect getVisibleRect() const;
    ChatLineContent* getContentFromPos(QPointF scenePos) const;

    qreal layout(int start, int end, qreal width);
    bool isOverSelection(QPointF scenePos) const;
    bool stickToBottom() const;

    qreal useableWidth() const;

    void reposition(int start, int end, qreal deltaY);
    void updateSceneRect();
    void partialUpdate();
    void fullUpdate();
    void checkVisibility();
    void scrollToBottom();

    virtual void mousePressEvent(QMouseEvent* ev);
    virtual void mouseReleaseEvent(QMouseEvent* ev);
    virtual void mouseMoveEvent(QMouseEvent* ev);
    virtual void scrollContentsBy(int dx, int dy);
    virtual void resizeEvent(QResizeEvent *ev);

    void updateMultiSelectionRect();
    void updateTypingNotification();

    ChatLine::Ptr findLineByYPos(qreal yPos) const;

private slots:
    void onSelectionTimerTimeout();

private:
    enum SelectionMode {
        None,
        Precise,
        Multi,
    };

    enum AutoScrollDirection {
        NoDirection,
        Up,
        Down,
    };

    QGraphicsScene* scene = nullptr;
    QVector<ChatLine::Ptr> lines;
    QList<ChatLine::Ptr> visibleLines;
    ChatLine::Ptr typingNotification;

    // selection
    int selClickedRow = -1; //These 4 are only valid while selectionMode != None
    int selClickedCol = -1;
    int selFirstRow = -1;
    int selLastRow = -1;
    SelectionMode selectionMode = None;
    QPointF clickPos;
    QGraphicsRectItem* selGraphItem = nullptr;
    QTimer* selectionTimer = nullptr;
    QTimer* workerTimer = nullptr;
    AutoScrollDirection selectionScrollDir = NoDirection;

    //worker vars
    int workerLastIndex = 0;
    qreal workerDy = 0;
    bool workerStb = false;

    // actions
    QAction* copyAction = nullptr;

    // layout
    QMarginsF margins = QMarginsF(10.0,10.0,10.0,10.0);
    qreal lineSpacing = 5.0f;

};

#endif // CHATLOG_H
