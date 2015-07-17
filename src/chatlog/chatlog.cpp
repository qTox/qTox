/*
    Copyright © 2014-2015 by The qTox Project

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

#include "chatlog.h"
#include "chatmessage.h"
#include "chatlinecontent.h"
#include "content/timestamp.h"
#include "src/widget/translator.h"
#include "src/persistence/settings.h"
#include "src/widget/notificationedgewidget.h"

#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QTimer>
#include <QMouseEvent>
#include <QShortcut>

template<class T>
T clamp(T x, T min, T max)
{
    if (x > max)
        return max;
    if (x < min)
        return min;
    return x;
}

ChatLog::ChatLog(QWidget* parent)
    : QGraphicsView(parent)
{
    // Create the scene
    busyScene = new QGraphicsScene(this);
    scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);
    setScene(scene);

    // Cfg.
    setInteractive(true);
    setAcceptDrops(false);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::NoDrag);
    setViewportUpdateMode(MinimalViewportUpdate);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setBackgroundBrush(QBrush(Qt::white, Qt::SolidPattern));

    // The selection rect for multi-line selection
    selGraphItem = scene->addRect(0,0,0,0,selectionRectColor.darker(120),selectionRectColor);
    selGraphItem->setZValue(-1.0); // behind all other items

    // copy action (ie. Ctrl+C)
    copyAction = new QAction(this);
    copyAction->setIcon(QIcon::fromTheme("edit-copy"));
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setEnabled(false);
    connect(copyAction, &QAction::triggered, this, [this]() { copySelectedText(); });
    addAction(copyAction);

#ifdef Q_OS_LINUX
    // Ctrl+Insert shortcut
    QShortcut* copyCtrlInsShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Insert), this);
    connect(copyCtrlInsShortcut, &QShortcut::activated, this, [this]() { copySelectedText(); });
#endif

    // select all action (ie. Ctrl+A)
    selectAllAction = new QAction(this);
    selectAllAction->setIcon(QIcon::fromTheme("edit-select-all"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, [this]() { selectAll(); });
    addAction(selectAllAction);

    // This timer is used to scroll the view while the user is
    // moving the mouse past the top/bottom edge of the widget while selecting.
    selectionTimer = new QTimer(this);
    selectionTimer->setInterval(1000/30);
    selectionTimer->setSingleShot(false);
    selectionTimer->start();
    connect(selectionTimer, &QTimer::timeout, this, &ChatLog::onSelectionTimerTimeout);

    // Background worker
    // Updates the layout of all chat-lines after a resize
    workerTimer = new QTimer(this);
    workerTimer->setSingleShot(false);
    workerTimer->setInterval(5);
    connect(workerTimer, &QTimer::timeout, this, &ChatLog::onWorkerTimeout);

    // selection
    connect(this, &ChatLog::selectionChanged, this, [this]() {
        copyAction->setEnabled(hasTextToBeCopied());
#ifdef Q_OS_LINUX
        copySelectedText(true);
#endif
    });

    retranslateUi();
    Translator::registerHandler(std::bind(&ChatLog::retranslateUi, this), this);

    // Place holders. They will be changed by layout().
    globalDateMessage = ChatMessage::createChatInfoMessage("", ChatMessage::ERROR, QDateTime());
    globalDateRect = scene->addRect(globalDateMessage->sceneBoundingRect(), Qt::NoPen, Qt::white);
    globalDateRect->setZValue(1);
    globalDateIndex = 0;
}

ChatLog::~ChatLog()
{
    Translator::unregister(this);

    // Remove chatlines from scene
    for (ChatLine::Ptr l : lines)
        l->removeFromScene();

    if (busyNotification)
        busyNotification->removeFromScene();

    if (typingNotification)
        typingNotification->removeFromScene();
}

void ChatLog::clearSelection()
{
    if (!(selectionMode & Selected))
        return;

    for (int i=selFirstRow; i<=selLastRow; ++i)
        lines[i]->selectionCleared();

    selFirstRow = -1;
    selLastRow = -1;
    selClickedCol = -1;
    selClickedRow = -1;

    selectionMode = None;
    emit selectionChanged();

    updateMultiSelectionRect();
}

QRect ChatLog::getVisibleRect() const
{
    return mapToScene(viewport()->rect()).boundingRect().toRect();
}

void ChatLog::updateSceneRect()
{
    setSceneRect(calculateSceneRect());
}

void ChatLog::layout(int start, int end, qreal width)
{
    onScrollBarChanged(verticalScrollBar()->value());

    if (lines.empty())
        return;

    qreal h = 0.0;

    // Line at start-1 is considered to have the correct position. All following lines are
    // positioned in respect to this line.
    if (start - 1 >= 0)
        h = lines[start - 1]->sceneBoundingRect().bottom() + lineSpacing;

    start = clamp<int>(start, 0, lines.size());
    end = clamp<int>(end + 1, 0, lines.size());

    for (int i = start; i < end; ++i)
    {
        ChatLine* l = lines[i].get();

        l->layout(width, QPointF(0.0, h));
        h += l->sceneBoundingRect().height() + lineSpacing;
    }
}

void ChatLog::mousePressEvent(QMouseEvent* ev)
{
    QGraphicsView::mousePressEvent(ev);

    if (ev->button() == Qt::LeftButton)
    {
        clickPos = ev->pos();
        clearSelection();
    }
}

void ChatLog::mouseReleaseEvent(QMouseEvent* ev)
{
    QGraphicsView::mouseReleaseEvent(ev);

    viewport()->unsetCursor();
    selectionScrollDir = NoDirection;

    mouseMoveEvent(ev); // To fix the cursor.
}

void ChatLog::mouseMoveEvent(QMouseEvent* ev)
{
    QGraphicsView::mouseMoveEvent(ev);

    QPointF scenePos = mapToScene(ev->pos());

    int leftColumnWidth = Settings::getInstance().getColumnLeftWidth();
    int rightColumnWidth = Settings::getInstance().getColumnRightWidth();
    int endLocation = width() - rightColumnWidth - margins.left() - verticalScrollBar()->sizeHint().width() - 15 / 2;

    bool splitterLeft = scenePos.x() > leftColumnWidth && scenePos.x() < leftColumnWidth + 15;
    bool splitterRight = scenePos.x() > endLocation - 15 && scenePos.x() < endLocation;

    if ((splitterLeft || splitterRight) && selectionMode == None)
    {
        viewport()->setCursor(Qt::SplitHCursor);

        if (ev->buttons() & Qt::LeftButton)
        {
            if (splitterLeft)
            {
                selectionMode = SplitterLeft;
                splitterVal = leftColumnWidth;
            }
            else
            {
                selectionMode = SplitterRight;
                splitterVal = rightColumnWidth;
            }
        }
    }
    else if (viewport()->cursor().shape() == Qt::SplitHCursor)
    {
        viewport()->unsetCursor();
    }

    if (selectionMode & Splitter)
    {
        if (!(ev->buttons() & Qt::LeftButton))
        {
            selectionMode = None;
        }
    }


    if (ev->buttons() & Qt::LeftButton)
    {
        if (selectionMode & Splitter)
        {
            viewport()->setCursor(Qt::SplitHCursor);
            int previousWidth = splitterVal;
            int newWidth;

            if (selectionMode == SplitterLeft)
            {
                newWidth = clamp<int>(splitterVal - (clickPos.x() - ev->pos().x()), 32, 320);
                Settings::getInstance().setColumnLeftWidth(newWidth);
            }
            else
            {
                newWidth = clamp<int>(splitterVal + (clickPos.x() - ev->pos().x()), 32, 320);
                Settings::getInstance().setColumnRightWidth(newWidth);
            }

            if (newWidth == 32 || newWidth == 320)
            {
                splitterVal = newWidth;
                clickPos.setX(ev->pos().x());
            }

            updateLayout(newWidth, previousWidth);
            return;
        }

        //autoscroll
        if (ev->pos().y() < 0)
            selectionScrollDir = Up;
        else if (ev->pos().y() > height())
            selectionScrollDir = Down;
        else
            selectionScrollDir = NoDirection;

        //select
        if (selectionMode == None && (clickPos - ev->pos()).manhattanLength() > QApplication::startDragDistance())
        {
            QPointF sceneClickPos = mapToScene(clickPos.toPoint());
            ChatLine::Ptr line = findLineByPosY(scenePos.y());

            ChatLineContent* content = getContentFromPos(sceneClickPos);
            if (content)
            {
                selClickedRow = content->getRow();
                selClickedCol = content->getColumn();
                selFirstRow = content->getRow();
                selLastRow = content->getRow();

                content->selectionStarted(sceneClickPos);

                selectionMode = Precise;

                // ungrab mouse grabber
                if (scene->mouseGrabberItem())
                    scene->mouseGrabberItem()->ungrabMouse();
            }
            else if (line.get())
            {
                selClickedRow = line->getRow();
                selFirstRow = selClickedRow;
                selLastRow = selClickedRow;

                selectionMode = Multi;
            }
        }

        if (selectionMode & Selected)
        {
            ChatLineContent* content = getContentFromPos(scenePos);
            ChatLine::Ptr line = findLineByPosY(scenePos.y());

            int row;

            if (content)
            {
                row = content->getRow();
                int col = content->getColumn();

                if (row == selClickedRow && col == selClickedCol)
                {
                    selectionMode = Precise;

                    content->selectionMouseMove(scenePos);
                    selGraphItem->hide();
                }
                else if (col != selClickedCol)
                {
                    selectionMode = Multi;

                    lines[selClickedRow]->selectionCleared();
                }
            }
            else if (line.get())
            {
                row = line->getRow();

                if (row != selClickedRow)
                {
                    selectionMode = Multi;

                    lines[selClickedRow]->selectionCleared();
                }

            }
            else
                return;

            if (row >= selClickedRow)
                selLastRow = row;

            if (row <= selClickedRow)
                selFirstRow = row;

            updateMultiSelectionRect();
            viewport()->setCursor(Qt::IBeamCursor);
        }
        emit selectionChanged();
    }
}

//Much faster than QGraphicsScene::itemAt()!
ChatLineContent* ChatLog::getContentFromPos(QPointF scenePos) const
{
    if (lines.empty())
        return nullptr;

    auto itr = std::lower_bound(lines.cbegin(), lines.cend(), scenePos.y(), ChatLine::lessThanBSRectBottom);

    //find content
    if (itr != lines.cend() && (*itr)->sceneBoundingRect().contains(scenePos))
        return (*itr)->getContent(scenePos);

    return nullptr;
}

bool ChatLog::isOverSelection(QPointF scenePos) const
{
    if (selectionMode == Precise)
    {
        ChatLineContent* content = getContentFromPos(scenePos);

        if (content)
            return content->isOverSelection(scenePos);
    }
    else if (selectionMode == Multi)
    {
        if (selGraphItem->rect().contains(scenePos))
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
    if (lines.isEmpty())
        return;

    start = clamp<int>(start, 0, lines.size() - 1);
    end = clamp<int>(end + 1, 0, lines.size());

    for (int i = start; i < end; ++i)
    {
        ChatLine* l = lines[i].get();
        l->moveBy(deltaY);
    }
}

void ChatLog::setVerticalScrollBar(QScrollBar* scrollbar)
{
    QGraphicsView::setVerticalScrollBar(scrollbar);
    onScrollBarChanged(verticalScrollBar()->value());

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &ChatLog::onScrollBarChanged);
}

void ChatLog::insertChatlineAtBottom(ChatLine::Ptr l)
{
    if (!l.get())
        return;

    bool stickToBtm = stickToBottom();

    // check for date.
    if (getLatestDate() < QDate::currentDate())
    {
        ChatMessage::Ptr date = ChatMessage::createChatInfoMessage(QDate::currentDate().toString(Settings::getInstance().getDateFormat()), ChatMessage::INFO);
        date->setRow(lines.size());

        if (!date->getContent(0)->scene())
            date->addToScene(scene);

        lines.append(date);

        dateMessages.push_front(QPair<QDate, ChatMessage::Ptr>(QDate::currentDate(), date));
        layout(lines.last()->getRow(), lines.size(), useableWidth());
    }

    //insert
    l->setRow(lines.size());
    l->addToScene(scene);
    lines.append(l);

    //partial refresh
    layout(lines.last()->getRow(), lines.size(), useableWidth());
    updateSceneRect();

    if (stickToBtm)
        scrollToBottom();

    checkVisibility();
    updateTypingNotification();

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &ChatLog::onScrollBarChanged);
    onScrollBarChanged(verticalScrollBar()->value());
}

void ChatLog::insertChatlineOnTop(ChatLine::Ptr l)
{
    if (!l.get())
        return;

    insertChatlineOnTop(QList<ChatLine::Ptr>() << l);
}

void ChatLog::insertChatlineOnTop(const QList<ChatLine::Ptr>& newLines)
{
    if (newLines.isEmpty())
        return;

    QGraphicsScene::ItemIndexMethod oldIndexMeth = scene->itemIndexMethod();
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    // alloc space for old and new lines
    QVector<ChatLine::Ptr> combLines;
    combLines.reserve(newLines.size() + lines.size());

    // add the new lines
    int i = 0;
    for (ChatLine::Ptr l : newLines)
    {
        l->addToScene(scene);
        l->visibilityChanged(false);
        l->setRow(i++);
        combLines.push_back(l);
    }

    // add the old lines
    for (ChatLine::Ptr l : lines)
    {
        l->setRow(i++);
        combLines.push_back(l);
    }

    lines = combLines;

    scene->setItemIndexMethod(oldIndexMeth);

    // redo layout
    startResizeWorker();

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &ChatLog::onScrollBarChanged);
    onScrollBarChanged(verticalScrollBar()->value());
}

void ChatLog::showNotification(ChatLine::Ptr l)
{
    if (!stickToBottom() && !edgeWidget)
    {
        edgeWidget = new NotificationEdgeWidget(NotificationEdgeWidget::Bottom, this);
        connect(edgeWidget, &NotificationEdgeWidget::clicked, this, &ChatLog::focusNotifiedWidget);
        recalculateNotificationEdge();
        edgeWidget->show();
        edgeWidget->updateNotificationCount(0);

        edgeMessage = l;
    }
}

bool ChatLog::stickToBottom() const
{
    return verticalScrollBar()->value() == verticalScrollBar()->maximum();
}

void ChatLog::scrollToBottom()
{
    updateSceneRect();
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    onScrollBarChanged(verticalScrollBar()->value());
}

void ChatLog::startResizeWorker()
{
    if (lines.empty())
        return;

    // (re)start the worker
    if (!workerTimer->isActive())
    {
        // these values must not be reevaluated while the worker is running
        workerStb = stickToBottom();

        if (!visibleLines.empty())
            workerAnchorLine = visibleLines.first();
    }

    // switch to busy scene displaying the busy notification if there is a lot
    // of text to be resized
    int txt = 0;
    for (ChatLine::Ptr line : lines)
    {
        if (txt > 500000)
            break;
        for (ChatLineContent* content : line->content)
            txt += content->getText().size();
    }
    if (txt > 500000)
        setScene(busyScene);

    workerLastIndex = 0;
    workerTimer->start();

    verticalScrollBar()->hide();
    disconnect(verticalScrollBar(), &QScrollBar::valueChanged, this, &ChatLog::onScrollBarChanged);
    onScrollBarChanged(verticalScrollBar()->value());
}

void ChatLog::mouseDoubleClickEvent(QMouseEvent *ev)
{
    QPointF scenePos = mapToScene(ev->pos());
    ChatLineContent* content = getContentFromPos(scenePos);

    if (content)
    {
        content->selectionDoubleClick(scenePos);
        selClickedCol = content->getColumn();
        selClickedRow = content->getRow();
        selFirstRow = content->getRow();
        selLastRow = content->getRow();
        selectionMode = Precise;

        emit selectionChanged();
    }
}

QString ChatLog::getSelectedText() const
{
    if (selectionMode == Precise)
    {
        return lines[selClickedRow]->content[selClickedCol]->getSelectedText();
    }
    else if (selectionMode == Multi)
    {
        // build a nicely formatted message
        QString out;

        for (int i=selFirstRow; i<=selLastRow; ++i)
        {
            if (lines[i]->content[1]->getText().isEmpty())
                continue;

            QString timestamp = lines[i]->content[2]->getText().isEmpty() ? tr("pending") : lines[i]->content[2]->getText();
            QString author = lines[i]->content[0]->getText();
            QString msg = lines[i]->content[1]->getText();

            out += QString(out.isEmpty() ? "[%2] %1: %3" : "\n[%2] %1: %3").arg(author, timestamp, msg);
        }

        return out;
    }

    return QString();
}

int ChatLog::findText(const QString &text, Qt::CaseSensitivity sensitivity, int &index)
{
    clearSelection();
    foundText.clear();

    index = -1;
    int i = 0;
    ChatLine::Ptr toSelect;
    ChatLine::Ptr first;

    for (ChatLine::Ptr chatLine : lines)
    {
        int last = i;
        i += chatLine.get()->setHighlight(text, sensitivity);

        if (last != i)
        {
            if (!first)
                first = chatLine;

            for (int iter = last; iter != i; ++iter)
                foundText.insert(iter, chatLine);

            bool above = chatLine.get()->sceneBoundingRect().top() >= verticalScrollBar()->value();

            if (index == -1 && above)
            {
                index = last;
                toSelect = chatLine;
            }
        }

        chatLine.get()->selectionCleared();
    }

    // If we didn't find anything near the scroll area, then go to the first found.
    if (index == -1 && i != 0)
    {
        index = 0;
        toSelect = first;
    }

    if (index != -1)
    {
        ensureVisible(toSelect.get()->sceneBoundingRect());

        selClickedCol = toSelect->selectNext(text, sensitivity);
        selClickedRow = toSelect->getRow();
        selFirstRow = toSelect->getRow();
        selLastRow = toSelect->getRow();
        selectionMode = Precise;
    }

    update();
    emit selectionChanged();

    return i;
}

int ChatLog::findNext(const QString &text, int to, int total, Qt::CaseSensitivity sensitivity)
{
    bool clear = true;
    int next = to;
    auto newFound = foundText.find(next - 1);
    auto lastFound = foundText.find(next - 2);

    if (lastFound == foundText.end())
        lastFound = foundText.find(total - 1);

    if (newFound == foundText.end())
    {
        newFound = foundText.find(0);
        next = 1;
    }

    if (newFound.value() == lastFound.value())
        clear = false;

    if (clear)
        lastFound.value().get()->selectionCleared();

    ensureVisible(newFound.value().get()->sceneBoundingRect());

    if (total != 1 || getSelectedText() != text)
        newFound.value().get()->selectNext(text, sensitivity);

    update();
    return next;
}

int ChatLog::findPrevious(const QString &text, int to, int total, Qt::CaseSensitivity sensitivity)
{
    bool clear = true;
    int next = to;
    auto newFound = foundText.find(next - 1);
    auto lastFound = foundText.find(next);

    if (lastFound == foundText.end())
        lastFound = foundText.find(0);

    if (newFound == foundText.end())
    {
        newFound = foundText.find(total - 1);
        next = total;
    }

    if (newFound.value() == lastFound.value())
        clear = false;

    if (clear)
        lastFound.value().get()->selectionCleared();

    ensureVisible(newFound.value().get()->sceneBoundingRect());

    if (total != 1 || getSelectedText() != text)
        newFound.value().get()->selectPrevious(text, sensitivity);

    update();
    return next;
}

const QHash<int, ChatLine::Ptr>& ChatLog::getFoundLines() const
{
    return foundText;
}

bool ChatLog::isEmpty() const
{
    return lines.isEmpty();
}

bool ChatLog::hasTextToBeCopied() const
{
    return selectionMode & Selected;
}

void ChatLog::addDateMessage(QDate date, ChatMessage::Ptr message)
{
    // Don't keep duplicate dates. Remove old, keep new.
    if (!dateMessages.isEmpty() && date == dateMessages.last().first)
    {
        dateMessages.last().second->removeFromScene();
        dateMessages.pop_back();

        if (!lines.isEmpty())
        {
            lines.pop_front();
            checkVisibility();
        }
    }

    dateMessages.push_back(QPair<QDate, ChatMessage::Ptr>(date, message));
}

ChatLine::Ptr ChatLog::getTypingNotification() const
{
    return typingNotification;
}

QVector<ChatLine::Ptr> ChatLog::getLines()
{
    return lines;
}

ChatLine::Ptr ChatLog::getLatestLine() const
{
    if (!lines.empty())
    {
        return lines.last();
    }
    return nullptr;
}

QDate ChatLog::getLatestDate() const
{
    int index = lines.size() - 1;

    while (index > 0)
    {
        Timestamp* timestamp = dynamic_cast<Timestamp*>(lines[index--]->getContent(2));

        if (timestamp)
            return timestamp->getTime().date();
    }

    return QDate();
}

void ChatLog::clear()
{
    clearSelection();

    for (ChatLine::Ptr l : lines)
        l->removeFromScene();

    lines.clear();
    visibleLines.clear();

    dateMessages.clear();
    globalDateIndex = 0;

    updateSceneRect();
}

void ChatLog::copySelectedText(bool toSelectionBuffer) const
{
    QString text = getSelectedText();
    QClipboard* clipboard = QApplication::clipboard();

    if (clipboard && !text.isNull())
        clipboard->setText(text, toSelectionBuffer ? QClipboard::Selection : QClipboard::Clipboard);
}

void ChatLog::setBusyNotification(ChatLine::Ptr notification)
{
    if (!notification.get())
        return;

    busyNotification = notification;
    busyNotification->addToScene(busyScene);
    busyNotification->visibilityChanged(true);
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
    if (typingNotification.get())
    {
        typingNotification->setVisible(visible);
        updateTypingNotification();
    }
}

void ChatLog::scrollToLine(ChatLine::Ptr line)
{
    if (!line.get())
        return;

    updateSceneRect();
    verticalScrollBar()->setValue(line->sceneBoundingRect().top());
}

void ChatLog::selectAll()
{
    if (lines.empty())
        return;

    clearSelection();

    selectionMode = Multi;
    selFirstRow = 0;
    selLastRow = lines.size()-1;

    emit selectionChanged();
    updateMultiSelectionRect();
}

void ChatLog::forceRelayout()
{
    startResizeWorker();
}

void ChatLog::checkVisibility()
{
    if (lines.empty())
        return;

    // find first visible line
    auto lowerBound = std::lower_bound(lines.cbegin(), lines.cend(), getVisibleRect().top(), ChatLine::lessThanBSRectBottom);

    // find last visible line
    auto upperBound = std::lower_bound(lowerBound, lines.cend(), getVisibleRect().bottom(), ChatLine::lessThanBSRectTop);

    // set visibilty
    QList<ChatLine::Ptr> newVisibleLines;
    for (auto itr = lowerBound; itr != upperBound; ++itr)
    {
        newVisibleLines.append(*itr);

        if (!visibleLines.contains(*itr))
            (*itr)->visibilityChanged(true);

        visibleLines.removeOne(*itr);
    }

    // these lines are no longer visible
    for (ChatLine::Ptr line : visibleLines)
        line->visibilityChanged(false);

    visibleLines = newVisibleLines;

    // enforce order
    std::sort(visibleLines.begin(), visibleLines.end(), ChatLine::lessThanRowIndex);

    //if (!visibleLines.empty())
    //  qDebug() << "visible from " << visibleLines.first()->getRow() << "to " << visibleLines.last()->getRow() << " total " << visibleLines.size();
}

void ChatLog::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    checkVisibility();
}

void ChatLog::resizeEvent(QResizeEvent* ev)
{
    updateLayout(ev->size().width(), ev->oldSize().width());

    if (edgeWidget)
        recalculateNotificationEdge();

    QGraphicsView::resizeEvent(ev);
}

void ChatLog::updateMultiSelectionRect()
{
    if (selectionMode == Multi && selFirstRow >= 0 && selLastRow >= 0)
    {
        QRectF selBBox;
        selBBox = selBBox.united(lines[selFirstRow]->sceneBoundingRect());
        selBBox = selBBox.united(lines[selLastRow]->sceneBoundingRect());

        if (selGraphItem->rect() != selBBox)
            scene->invalidate(selGraphItem->rect());

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
    if (!notification)
        return;

    qreal posY = 0.0;

    if (!lines.empty())
        posY = lines.last()->sceneBoundingRect().bottom() + lineSpacing;

    notification->layout(useableWidth(), QPointF(0.0, posY));
}

void ChatLog::updateBusyNotification()
{
    if (busyNotification.get())
    {
        //repoisition the busy notification (centered)
        busyNotification->layout(useableWidth(), getVisibleRect().topLeft() + QPointF(0, getVisibleRect().height()/2.0));
    }
}

ChatLine::Ptr ChatLog::findLineByPosY(qreal yPos) const
{
    auto itr = std::lower_bound(lines.cbegin(), lines.cend(), yPos, ChatLine::lessThanBSRectBottom);

    if (itr != lines.cend())
        return *itr;

    return ChatLine::Ptr();
}

QRectF ChatLog::calculateSceneRect() const
{
    qreal bottom = (lines.empty() ? 0.0 : lines.last()->sceneBoundingRect().bottom());

    if (typingNotification.get() != nullptr)
        bottom += typingNotification->sceneBoundingRect().height() + lineSpacing;

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

void ChatLog::onWorkerTimeout()
{
    // Fairly arbitrary but
    // large values will make the UI unresponsive
    const int stepSize = 50;

    layout(workerLastIndex, workerLastIndex+stepSize, useableWidth());
    workerLastIndex += stepSize;

    // done?
    if (workerLastIndex >= lines.size())
    {
        workerTimer->stop();

        // switch back to the scene containing the chat messages
        setScene(scene);

        // make sure everything gets updated
        updateSceneRect();
        checkVisibility();
        updateTypingNotification();
        updateMultiSelectionRect();

        // scroll
        if (workerStb)
            scrollToBottom();
        else
            scrollToLine(workerAnchorLine);

        // don't keep a Ptr to the anchor line
        workerAnchorLine = ChatLine::Ptr();

        // hidden during busy screen
        verticalScrollBar()->show();
        connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &ChatLog::onScrollBarChanged);
    }
}

void ChatLog::onScrollBarChanged(int value)
{
    if (stickToBottom())
        removeNotificationWidget();

    if (dateMessages.isEmpty())
    {
        globalDateMessage->layout(width(), QPointF(0.0, value));// - lineSpacing));
        return;
    }

    if (verticalScrollBar()->maximum() != verticalScrollBar()->minimum())
        value += margins.top();

    if (dateMessages.count() != 0)
    {
        // Find a closer date.
        while (globalDateIndex < dateMessages.count() - 1)
        {
            if (dateMessages[globalDateIndex].second->getContent(1)->y() > value)
                ++globalDateIndex;
            else
                break;
        }

        // Find a closer date.
        while (globalDateIndex > 0)
        {
            int y = dateMessages[globalDateIndex - 1].second->getContent(1)->y();

            if (y < value)
            {
                --globalDateIndex;
            }
            else
            {
                int height = dateMessages[globalDateIndex - 1].second->sceneBoundingRect().height() + lineSpacing;

                if (value > y - height)
                    value = y - height;
                break;
            }
        }

        globalDateMessage->removeFromScene();
        globalDateMessage = ChatMessage::createChatInfoMessage(dateMessages[globalDateIndex].first.toString(Settings::getInstance().getDateFormat()), ChatMessage::INFO, QDateTime());
        globalDateMessage->addToScene(scene);
        globalDateMessage->getContent(0)->setZValue(2);
        globalDateMessage->getContent(1)->setZValue(2);
        globalDateMessage->getContent(2)->setZValue(2);
        globalDateMessage->visibilityChanged(true);
    }

    globalDateMessage->layout(width(), QPointF(0.0, value));// - lineSpacing));
    QRectF globalDateSceneRect = globalDateMessage->sceneBoundingRect();
    globalDateSceneRect.setY(globalDateSceneRect.y() - margins.top() - lineSpacing);
    globalDateRect->setRect(globalDateSceneRect);
}

void ChatLog::focusNotifiedWidget()
{
    scrollToLine(edgeMessage);
    removeNotificationWidget();
}

void ChatLog::removeNotificationWidget()
{
    if (edgeWidget)
    {
        edgeWidget->deleteLater();
        edgeWidget = nullptr;
    }

    edgeMessage.reset();
}

void ChatLog::showEvent(QShowEvent*)
{
    // Empty.
    // The default implementation calls centerOn - for some reason - causing
    // the scrollbar to move.
}

void ChatLog::focusInEvent(QFocusEvent* ev)
{
    QGraphicsView::focusInEvent(ev);

    if (selectionMode & Selected)
    {
        selGraphItem->setBrush(QBrush(selectionRectColor));

        for (int i=selFirstRow; i<=selLastRow; ++i)
            lines[i]->selectionFocusChanged(true);
    }
}

void ChatLog::focusOutEvent(QFocusEvent* ev)
{
    QGraphicsView::focusOutEvent(ev);

    if (selectionMode & Selected)
    {
        selGraphItem->setBrush(QBrush(selectionRectColor.lighter(120)));

        for (int i=selFirstRow; i<=selLastRow; ++i)
            lines[i]->selectionFocusChanged(false);
    }
}

void ChatLog::retranslateUi()
{
    copyAction->setText(tr("Copy"));
    selectAllAction->setText(tr("Select all"));
}

void ChatLog::updateLayout(int currentWidth, int previousWidth)
{
    bool stb = stickToBottom();

    if (currentWidth != previousWidth)
    {
        startResizeWorker();
        stb = false; // let the resize worker handle it
    }

    if (stb)
        scrollToBottom();

    updateBusyNotification();
}

void ChatLog::recalculateNotificationEdge()
{
    QPoint position = viewport()->pos();
    position.setY(position.y() + viewport()->height() - edgeWidget->height());
    edgeWidget->move(position);
    edgeWidget->resize(viewport()->width(), edgeWidget->height());
}
