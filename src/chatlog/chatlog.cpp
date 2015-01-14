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

#include "chatlog.h"
#include "chatmessage.h"
#include "chatlinecontent.h"

#include <QDebug>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QTimer>

template<class T>
T clamp(T x, T min, T max)
{
    if(x > max)
        return max;
    if(x < min)
        return min;
    return x;
}

ChatLog::ChatLog(QWidget* parent)
    : QGraphicsView(parent)
{
    // Create the scene
    scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex); //Bsp-Tree is actually slower in this case
    setScene(scene);

    // Cfg.
    setInteractive(true);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::NoDrag);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setAcceptDrops(false);
    setContextMenuPolicy(Qt::CustomContextMenu);

    // The selection rect for multi-line selection
    const QColor selGraphColor = QColor(166,225,255);
    selGraphItem = scene->addRect(0,0,0,0,selGraphColor.darker(120),selGraphColor);
    selGraphItem->setZValue(-10.0); //behind all items

    // copy action (ie. Ctrl+C)
    copyAction = new QAction(this);
    copyAction->setShortcut(QKeySequence::Copy);
    addAction(copyAction);
    connect(copyAction, &QAction::triggered, this, [ = ](bool)
    {
        copySelectedText();
    });

    // This timer is used to scroll the view while the user is
    // moving the mouse past the top/bottom edge of the widget while selecting.
    selectionTimer = new QTimer(this);
    selectionTimer->setInterval(1000/60);
    selectionTimer->setSingleShot(false);
    selectionTimer->start();
    connect(selectionTimer, &QTimer::timeout, this, &ChatLog::onSelectionTimerTimeout);

    // Background worker
    // Updates the layout of all chat-lines after a resize
    workerTimer = new QTimer(this);
    workerTimer->setSingleShot(false);
    workerTimer->setInterval(100);
    connect(workerTimer, &QTimer::timeout, this, [this] {
        const int stepSize = 400;

        workerDy += layout(workerLastIndex, workerLastIndex+stepSize, useableWidth());

        if(!visibleLines.isEmpty())
        {
            int firstVisLineIndex = visibleLines.first()->getRowIndex();
            int delta = firstVisLineIndex - workerLastIndex;
            if(delta > 0 && delta < stepSize)
            {
                workerLastIndex += delta+1;

                if(!workerStb)
                    verticalScrollBar()->setValue(verticalScrollBar()->value() - workerDy);

                workerDy = 0.0;
                checkVisibility();
            }
            else
            {
                workerLastIndex += stepSize;
            }
        }
        else
            workerLastIndex += stepSize;

        if(workerLastIndex >= lines.size())
        {
            workerTimer->stop();
            workerLastIndex = 0;
            workerDy = 0.0;

            if(stickToBottom())
                scrollToBottom();
        }
    });
}

void ChatLog::clearSelection()
{
    for(int i=selFirstRow; i<=selLastRow; ++i)
        lines[i]->selectionCleared();

    selFirstRow = -1;
    selLastRow = -1;
    selClickedCol = -1;
    selClickedRow = -1;

    selectionMode = None;

    updateMultiSelectionRect();
}

QRect ChatLog::getVisibleRect() const
{
    return mapToScene(calculateSceneRect().toRect()).boundingRect().toRect();
}

void ChatLog::updateSceneRect()
{
    setSceneRect(calculateSceneRect());
}

qreal ChatLog::layout(int start, int end, qreal width)
{
    if(lines.empty())
        return false;

    qreal h = 0.0;

    // Line at start-1 is considered to have the correct position. All following lines are
    // positioned in respect to this line.
    if(start - 1 >= 0)
        h = lines[start - 1]->boundingSceneRect().bottom() + lineSpacing;

    start = clamp<int>(start, 0, lines.size());
    end = clamp<int>(end + 1, 0, lines.size());

    qreal deltaY = 0.0;
    for(int i = start; i < end; ++i)
    {
        ChatLine* l = lines[i].get();

        qreal oldHeight = l->boundingSceneRect().height();
        l->layout(width, QPointF(0.0, h));

        if(oldHeight != l->boundingSceneRect().height())
            deltaY += oldHeight - l->boundingSceneRect().height();

        h += l->boundingSceneRect().height() + lineSpacing;
    }

    return deltaY;
}

void ChatLog::partialUpdate()
{
    checkVisibility();

    if(visibleLines.empty())
        return;

    bool stb = stickToBottom();

    auto oldUpdateMode = viewportUpdateMode();
    setViewportUpdateMode(NoViewportUpdate);

    // Resize all lines currently visible in the viewport.
    // If this creates some whitespace underneath the last visible lines
    // then move the following non visible lines up in order to fill the gap.
    qreal repos;
    do
    {
        repos = 0;
        if(!visibleLines.empty())
        {
            repos = layout(visibleLines.first()->getRowIndex(), visibleLines.last()->getRowIndex(), useableWidth());
            reposition(visibleLines.last()->getRowIndex()+1, visibleLines.last()->getRowIndex()+10, -repos);
            verticalScrollBar()->setValue(verticalScrollBar()->value() - repos);
        }

        checkVisibility();
    }
    while(repos != 0);

    checkVisibility();

    setViewportUpdateMode(oldUpdateMode);

    if(stb)
        scrollToBottom();
}

void ChatLog::fullUpdate()
{
    layout(0, lines.size(), useableWidth());
    checkVisibility();
}

void ChatLog::mousePressEvent(QMouseEvent* ev)
{
    QGraphicsView::mousePressEvent(ev);

    QPointF scenePos = mapToScene(ev->pos());

    if(ev->button() == Qt::LeftButton)
    {
        clickPos = ev->pos();
        clearSelection();
    }

    if(ev->button() == Qt::RightButton)
    {
        if(!isOverSelection(scenePos))
            clearSelection();
    }
}

void ChatLog::mouseReleaseEvent(QMouseEvent* ev)
{
    QGraphicsView::mouseReleaseEvent(ev);

    QPointF scenePos = mapToScene(ev->pos());

    if(ev->button() == Qt::RightButton)
    {
        if(!isOverSelection(scenePos))
            clearSelection();

        emit customContextMenuRequested(ev->pos());
    }

    selectionScrollDir = NoDirection;
}

void ChatLog::mouseMoveEvent(QMouseEvent* ev)
{
    QGraphicsView::mouseMoveEvent(ev);

    QPointF scenePos = mapToScene(ev->pos());

    if(ev->buttons() & Qt::LeftButton)
    {
        //autoscroll
        if(ev->pos().y() < 0)
            selectionScrollDir = Up;
        else if(ev->pos().y() > height())
            selectionScrollDir = Down;
        else
            selectionScrollDir = NoDirection;

        //select
        if(selectionMode == None && (clickPos - ev->pos()).manhattanLength() > QApplication::startDragDistance())
        {
            QPointF sceneClickPos = mapToScene(clickPos.toPoint());

            ChatLineContent* content = getContentFromPos(sceneClickPos);
            if(content)
            {
                selClickedRow = content->getRow();
                selClickedCol = content->getColumn();
                selFirstRow = content->getRow();
                selLastRow = content->getRow();

                content->selectionStarted(sceneClickPos);

                selectionMode = Precise;

                // ungrab mouse grabber
                if(scene->mouseGrabberItem())
                    scene->mouseGrabberItem()->ungrabMouse();
            }
        }

        if(selectionMode != None)
        {
            ChatLineContent* content = getContentFromPos(scenePos);

            if(content)
            {
                int row = content->getRow();
                int col = content->getColumn();

                if(row >= selClickedRow)
                    selLastRow = row;

                if(row <= selClickedRow)
                    selFirstRow = row;

                if(row == selClickedRow && col == selClickedCol)
                {
                    selectionMode = Precise;

                    content->selectionMouseMove(scenePos);
                    selGraphItem->hide();
                }
                else
                {
                    selectionMode = Multi;

                    lines[selClickedRow]->selectionCleared();

                    updateMultiSelectionRect();
                }
            }
        }
    }
}

ChatLineContent* ChatLog::getContentFromPos(QPointF scenePos) const
{
    if(lines.empty())
        return nullptr;

    //Much faster than QGraphicsScene::itemAt()!
    //the first visible line
    auto lowerBound = std::upper_bound(lines.cbegin(), lines.cend(), scenePos.y(), [](const qreal lhs, const ChatLine::Ptr rhs)
    {
        return lhs < rhs->boundingSceneRect().bottom();
    });

    //the last visible line
    auto upperBound = std::lower_bound(lines.cbegin(), lines.cend(), scenePos.y(), [](const ChatLine::Ptr lhs, const qreal rhs)
    {
        return lhs->boundingSceneRect().top() < rhs;
    });

    //find content
    for(auto itr = lowerBound; itr != upperBound; ++itr)
    {
        if((*itr)->boundingSceneRect().contains(scenePos))
            return (*itr)->getContent(scenePos);
    }

    return nullptr;
}

bool ChatLog::isOverSelection(QPointF scenePos) const
{
    if(selectionMode == Precise)
    {
        ChatLineContent* content = getContentFromPos(scenePos);

        if(content)
            return content->isOverSelection(scenePos);
    }
    else if(selectionMode == Multi)
    {
        if(selGraphItem->rect().contains(scenePos))
            return true;
    }

    return false;
}

qreal ChatLog::useableWidth() const
{
    return width() - verticalScrollBar()->sizeHint().width() - margins.right() - margins.left();
}

void ChatLog::reposition(int start, int end, qreal deltaY)
{
    if(lines.isEmpty())
        return;

    start = clamp<int>(start, 0, lines.size() - 1);
    end = clamp<int>(end + 1, 0, lines.size());

    for(int i = start; i < end; ++i)
    {
        ChatLine* l = lines[i].get();
        l->moveBy(deltaY);
    }
}

void ChatLog::insertChatlineAtBottom(ChatLine::Ptr l)
{
    if(!l.get())
        return;

    bool stickToBtm = stickToBottom();

    //insert
    l->setRowIndex(lines.size());
    l->addToScene(scene);
    lines.append(l);

    //partial refresh
    layout(lines.last()->getRowIndex(), lines.size(), useableWidth());
    updateSceneRect();

    if(stickToBtm)
        scrollToBottom();

    checkVisibility();
}

void ChatLog::insertChatlineOnTop(ChatLine::Ptr l)
{
    if(!l.get())
        return;

    bool stickToBtm = stickToBottom();

    //move all lines down by 1
    for(ChatLine::Ptr l : lines)
        l->setRowIndex(l->getRowIndex() + 1);

    //add the new line
    l->addToScene(scene);
    l->setRowIndex(0);
    lines.prepend(l);

    //full refresh is required
    layout(0, lines.size(), useableWidth());
    updateSceneRect();

    if(stickToBtm)
        scrollToBottom();

    checkVisibility();
}

void ChatLog::insertChatlineOnTop(const QList<ChatLine::Ptr>& newLines)
{
    if(newLines.isEmpty())
        return;

    bool stickToBtm = stickToBottom();

    //move all lines down by n
    int n = newLines.size();
    for(ChatLine::Ptr l : lines)
        l->setRowIndex(l->getRowIndex() + n);

    //add the new line
    for(ChatLine::Ptr l : newLines)
    {
        l->addToScene(scene);
        l->setRowIndex(--n);
        lines.prepend(l);
    }

    //full refresh is required
    layout(0, lines.size(), useableWidth());
    updateSceneRect();

    if(stickToBtm)
        scrollToBottom();

    checkVisibility();
}

bool ChatLog::stickToBottom() const
{
    return verticalScrollBar()->value() == verticalScrollBar()->maximum();
}

void ChatLog::scrollToBottom()
{
    updateSceneRect();
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

QString ChatLog::getSelectedText() const
{
    if(selectionMode == Precise)
    {
        return lines[selClickedRow]->content[selClickedCol]->getSelectedText();
    }
    else if(selectionMode == Multi)
    {
        // build a nicely formatted message
        QString out;

        QString lastSender;
        for(int i=selFirstRow; i<=selLastRow && i>=0 && i<lines.size(); ++i)
        {
            if(lastSender != lines[i]->content[0]->getText() && !lines[i]->content[0]->getText().isEmpty())
            {
                //author changed
                out += lines[i]->content[0]->getText() + ":\n";
                lastSender = lines[i]->content[0]->getText();
            }

            out += lines[i]->content[1]->getText();
            out += "\n\n";
        }

        return out;
    }

    return QString();
}

QString ChatLog::toPlainText() const
{
    QString out;

    for(ChatLine::Ptr l : lines)
    {
        out += QString("|%1 @%2|\n%3\n\n").arg(l->getContent(0)->getText(),l->getContent(2)->getText(),l->getContent(1)->getText());
    }

    return out;
}

bool ChatLog::isEmpty() const
{
    return lines.isEmpty();
}

bool ChatLog::hasTextToBeCopied() const
{
    return selectionMode != None;
}

ChatLine::Ptr ChatLog::getTypingNotification() const
{
    return typingNotification;
}

void ChatLog::clear()
{
    clearSelection();

    for(ChatLine::Ptr l : lines)
        l->removeFromScene();

    lines.clear();
    visibleLines.clear();

    updateSceneRect();
}

void ChatLog::copySelectedText() const
{
    QString text = getSelectedText();

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

void ChatLog::setTypingNotification(ChatLine::Ptr notification)
{
    typingNotification = notification;
    typingNotification->visibilityChanged(true);
    typingNotification->setVisible(false);
    typingNotification->addToScene(scene);
    updateTypingNotification();
}

void ChatLog::setTypingNotificationVisible(bool visible)
{
    if(typingNotification.get() != nullptr)
    {
        typingNotification->setVisible(visible);
        updateTypingNotification();
    }
}

void ChatLog::checkVisibility()
{
    if(lines.empty())
        return;

    // find first visible line
    auto lowerBound = std::upper_bound(lines.cbegin(), lines.cend(), getVisibleRect().top(), [](const qreal lhs, const ChatLine::Ptr rhs)
    {
        return lhs < rhs->boundingSceneRect().bottom();
    });

    // find last visible line
    auto upperBound = std::lower_bound(lines.cbegin(), lines.cend(), getVisibleRect().bottom(), [](const ChatLine::Ptr lhs, const qreal rhs)
    {
        return lhs->boundingSceneRect().top() < rhs;
    });

    // set visibilty
    QList<ChatLine::Ptr> newVisibleLines;
    for(auto itr = lowerBound; itr != upperBound; ++itr)
    {
        newVisibleLines.append(*itr);

        if(!visibleLines.contains(*itr))
            (*itr)->visibilityChanged(true);

        visibleLines.removeOne(*itr);
    }

    // these lines are no longer visible
    for(ChatLine::Ptr line : visibleLines)
        line->visibilityChanged(false);

    visibleLines = newVisibleLines;

    // enforce order
    std::sort(visibleLines.begin(), visibleLines.end(), [](const ChatLine::Ptr lhs, const ChatLine::Ptr rhs)
    {
        return lhs->getRowIndex() < rhs->getRowIndex();
    });

    //if(!visibleLines.empty())
    //    qDebug() << "visible from " << visibleLines.first()->getRowIndex() << "to " << visibleLines.last()->getRowIndex() << " total " << visibleLines.size();
}

void ChatLog::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    partialUpdate();
}

void ChatLog::resizeEvent(QResizeEvent* ev)
{
    if(!workerTimer->isActive())
    {
        workerStb = stickToBottom();
        workerLastIndex = 0;
        workerDy = 0.0;
        workerTimer->start();
    }

    bool stb = stickToBottom();

    QGraphicsView::resizeEvent(ev);

    if(lines.count() > 300)
        partialUpdate();
    else
        fullUpdate();

    if(stb)
        scrollToBottom();

    updateMultiSelectionRect();
}

void ChatLog::updateMultiSelectionRect()
{
    if(selectionMode == Multi && selFirstRow >= 0 && selLastRow >= 0)
    {
        QRectF selBBox;
        selBBox = selBBox.united(lines[selFirstRow]->boundingSceneRect());
        selBBox = selBBox.united(lines[selLastRow]->boundingSceneRect());

        selGraphItem->setRect(selBBox);
        selGraphItem->show();
    }
    else
    {
        selGraphItem->hide();
    }
}

void ChatLog::updateTypingNotification()
{
    ChatLine* notification = typingNotification.get();
    if(!notification)
        return;

    qreal posY = 0.0;

    if(!lines.empty())
        posY = lines.last()->boundingSceneRect().bottom() + lineSpacing;

    notification->layout(useableWidth(), QPointF(0.0, posY));
}

QRectF ChatLog::calculateSceneRect() const
{
    qreal bottom = (lines.empty() ? 0.0 : lines.last()->boundingSceneRect().bottom());

    if(typingNotification.get() != nullptr)
        bottom += typingNotification->boundingSceneRect().height() + lineSpacing;

    return QRectF(-margins.left(), -margins.top(), useableWidth(), bottom + margins.bottom() + margins.top());
}

void ChatLog::onSelectionTimerTimeout()
{
    const int scrollSpeed = 10;

    switch(selectionScrollDir)
    {
    case Up:
        verticalScrollBar()->setValue(verticalScrollBar()->value() - scrollSpeed);
        break;
    case Down:
        verticalScrollBar()->setValue(verticalScrollBar()->value() + scrollSpeed);
        break;
    default:
        break;
    }
}
