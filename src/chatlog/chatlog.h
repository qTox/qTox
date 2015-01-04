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
    virtual ~ChatLog();

    ChatMessage::Ptr addChatMessage(const QString& sender, const QString& msg, const QDateTime& timestamp, bool self, bool alert);
    ChatMessage::Ptr addChatMessage(const QString& sender, const QString& msg, bool self, bool alert);
    ChatMessage::Ptr addChatAction(const QString& sender, const QString& msg, const QDateTime& timestamp);
    ChatMessage::Ptr addChatAction(const QString& sender, const QString& msg);

    ChatMessage::Ptr addSystemMessage(const QString& msg, const QDateTime& timestamp);
    ChatMessage::Ptr addFileTransferMessage(const QString& sender, const ToxFile& file, const QDateTime &timestamp, bool self);

    void insertChatline(ChatLine::Ptr l);

    void clearSelection();
    void clear();
    void copySelectedText() const;
    QString getSelectedText() const;
    QString toPlainText() const;

    bool isEmpty() const;
    bool hasTextToBeCopied() const;

protected:
    QRect getVisibleRect() const;
    ChatLineContent* getContentFromPos(QPointF scenePos) const;

    bool layout(int start, int end, qreal width);
    bool isOverSelection(QPointF scenePos);
    bool stickToBottom();

    qreal useableWidth();

    void reposition(int start, int end);
    void repositionDownTo(int start, qreal end);
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
    QList<ChatLine::Ptr> lines;
    QList<ChatLine::Ptr> visibleLines;

    bool multiLineInsert = false;
    bool stickToBtm = false;
    int insertStartIndex = -1;

    // selection
    int selClickedRow = -1;
    int selClickedCol = -1;
    int selFirstRow = -1;
    int selLastRow = -1;
    SelectionMode selectionMode = None;
    QPointF clickPos;
    QPointF lastPos;
    QGraphicsRectItem* selGraphItem = nullptr;
    QTimer* selectionTimer = nullptr;
    AutoScrollDirection selectionScrollDir = NoDirection;

    // actions
    QAction* copyAction = nullptr;

    // layout
    QMarginsF margins = QMarginsF(10.0,10.0,10.0,10.0);
    qreal lineSpacing = 10.0f;

};

#endif // CHATLOG_H
