/*
    Copyright © 2014-2019 by The qTox Project Contributors

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

#include "chatwidget.h"
#include "chatlinecontent.h"
#include "chatlinecontentproxy.h"
#include "chatmessage.h"
#include "content/filetransferwidget.h"
#include "content/text.h"
#include "src/widget/translator.h"
#include "src/widget/style.h"
#include "src/persistence/settings.h"
#include "src/chatlog/chatlinestorage.h"
#include <iostream>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QShortcut>
#include <QTimer>

#include <algorithm>
#include <cassert>
#include <set>


namespace
{

// Maximum number of rendered messages at any given time
int constexpr maxWindowSize = 300;
// Amount of messages to purge when removing messages
int constexpr windowChunkSize = 100;

template <class T>
T clamp(T x, T min, T max)
{
    if (x > max)
        return max;
    if (x < min)
        return min;
    return x;
}

ChatMessage::Ptr createDateMessage(QDateTime timestamp, DocumentCache& documentCache,
    Settings& settings, Style& style)
{
    const auto date = timestamp.date();
    auto dateText = date.toString(settings.getDateFormat());
    return ChatMessage::createChatInfoMessage(dateText, ChatMessage::INFO, QDateTime(),
        documentCache, settings, style);
}

ChatMessage::Ptr createMessage(const QString& displayName, bool isSelf, bool colorizeNames,
                               const ChatLogMessage& chatLogMessage, DocumentCache& documentCache,
                               SmileyPack& smileyPack, Settings& settings, Style& style)
{
    auto messageType = chatLogMessage.message.isAction ? ChatMessage::MessageType::ACTION
                                                       : ChatMessage::MessageType::NORMAL;

    const bool bSelfMentioned =
        std::any_of(chatLogMessage.message.metadata.begin(), chatLogMessage.message.metadata.end(),
                    [](const MessageMetadata& metadata) {
                        return metadata.type == MessageMetadataType::selfMention;
                    });

    if (bSelfMentioned) {
        messageType = ChatMessage::MessageType::ALERT;
    }

    const auto timestamp = chatLogMessage.message.timestamp;
    return ChatMessage::createChatMessage(displayName, chatLogMessage.message.content,messageType,
                                          isSelf, chatLogMessage.state, timestamp, documentCache,
                                          smileyPack, settings, style, colorizeNames);
}

void renderMessageRaw(const QString& displayName, bool isSelf, bool colorizeNames,
                   const ChatLogMessage& chatLogMessage, ChatLine::Ptr& chatLine,
                   DocumentCache& documentCache, SmileyPack& smileyPack,
                   Settings& settings, Style& style)
{
    // HACK: This is kind of gross, but there's not an easy way to fit this into
    // the existing architecture. This shouldn't ever fail since we should only
    // correlate ChatMessages created here, however a logic bug could turn into
    // a crash due to this dangerous cast. The alternative would be to make
    // ChatLine a QObject which I didn't think was worth it.
    auto chatMessage = static_cast<ChatMessage*>(chatLine.get());

    if (chatMessage) {
        if (chatLogMessage.state == MessageState::complete) {
            chatMessage->markAsDelivered(chatLogMessage.message.timestamp);
        } else if (chatLogMessage.state == MessageState::broken) {
            chatMessage->markAsBroken();
        }
    } else {
        chatLine = createMessage(displayName, isSelf, colorizeNames, chatLogMessage,
            documentCache, smileyPack, settings, style);
    }
}

/**
 * @return Chat message message type (info/warning) for the given system message
 * @param[in] systemMessage
 */
ChatMessage::SystemMessageType getChatMessageType(const SystemMessage& systemMessage)
{
    switch (systemMessage.messageType)
    {
    case SystemMessageType::fileSendFailed:
    case SystemMessageType::messageSendFailed:
    case SystemMessageType::unexpectedCallEnd:
        return ChatMessage::ERROR;
    case SystemMessageType::userJoinedGroup:
    case SystemMessageType::userLeftGroup:
    case SystemMessageType::peerNameChanged:
    case SystemMessageType::peerStateChange:
    case SystemMessageType::titleChanged:
    case SystemMessageType::cleared:
    case SystemMessageType::outgoingCall:
    case SystemMessageType::incomingCall:
    case SystemMessageType::callEnd:
    case SystemMessageType::selfJoinedGroup:
    case SystemMessageType::selfLeftGroup:
        return ChatMessage::INFO;
    }

    return ChatMessage::INFO;
}

ChatLogIdx firstItemAfterDate(QDate date, const IChatLog& chatLog)
{
    auto idxs = chatLog.getDateIdxs(date, 1);
    if (idxs.size()) {
        return idxs[0].idx;
    } else {
        return chatLog.getNextIdx();
    }
}

/**
 * @brief Applies a function for each line in between first and last in storage.
 *
 * @note Undefined behavior if first appears after last
 * @note If first or last is not seen in storage it is assumed that they point
 * to elements past the edge of our rendered storage. We will iterate from the
 * beginning or to the end in these cases
 */
template <typename Fn>
void forEachLineIn(ChatLine::Ptr first, ChatLine::Ptr last, ChatLineStorage& storage, Fn f)
{
    auto startIt = storage.find(first);

    if (startIt == storage.end()) {
        startIt = storage.begin();
    }

    auto endIt = storage.find(last);
    if (endIt != storage.end()) {
        endIt++;
    }

    for (auto it = startIt; it != endIt; ++it) {
        f(*it);
    }
}

/**
 * @brief Helper function to add an offset ot a ChatLogIdx without going
 * outside the bounds of the associated chatlog
 */
ChatLogIdx clampedAdd(ChatLogIdx idx, int val, IChatLog& chatLog)
{
    if (val < 0) {
        auto distToEnd = idx - chatLog.getFirstIdx();
        if (static_cast<size_t>(std::abs(val)) > distToEnd) {
            return chatLog.getFirstIdx();
        }

        return idx - std::abs(val);
    } else {
        auto distToEnd = chatLog.getNextIdx() - idx;
        if (static_cast<size_t>(val) > distToEnd) {
            return chatLog.getNextIdx();
        }

        return idx + val;
    }
}

} // namespace


ChatWidget::ChatWidget(IChatLog& chatLog_, const Core& core_, DocumentCache& documentCache_,
    SmileyPack& smileyPack_, Settings& settings_, Style& style_,
    IMessageBoxManager& messageBoxManager_, QWidget* parent)
    : QGraphicsView(parent)
    , selectionRectColor{style_.getColor(Style::ColorPalette::SelectText)}
    , chatLog(chatLog_)
    , core(core_)
    , chatLineStorage(new ChatLineStorage())
    , documentCache(documentCache_)
    , smileyPack{smileyPack_}
    , settings(settings_)
    , style{style_}
    , messageBoxManager{messageBoxManager_}
{
    // Create the scene
    busyScene = new QGraphicsScene(this);
    scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);
    setScene(scene);

    busyNotification = ChatMessage::createBusyNotification(documentCache, settings, style);
    busyNotification->addToScene(busyScene);
    busyNotification->visibilityChanged(true);

    // Cfg.
    setInteractive(true);
    setAcceptDrops(false);
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::NoDrag);
    setViewportUpdateMode(MinimalViewportUpdate);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setBackgroundBrush(QBrush(style.getColor(Style::ColorPalette::GroundBase), Qt::SolidPattern));

    // The selection rect for multi-line selection
    selGraphItem = scene->addRect(0, 0, 0, 0, selectionRectColor.darker(120), selectionRectColor);
    selGraphItem->setZValue(-1.0); // behind all other items

    // copy action (ie. Ctrl+C)
    copyAction = new QAction(this);
    copyAction->setIcon(QIcon::fromTheme("edit-copy"));
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setEnabled(false);
    connect(copyAction, &QAction::triggered, this, [this]() { copySelectedText(); });
    addAction(copyAction);

    // Ctrl+Insert shortcut
    QShortcut* copyCtrlInsShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Insert), this);
    connect(copyCtrlInsShortcut, &QShortcut::activated, this, [this]() { copySelectedText(); });

    // select all action (ie. Ctrl+A)
    selectAllAction = new QAction(this);
    selectAllAction->setIcon(QIcon::fromTheme("edit-select-all"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, [this]() { selectAll(); });
    addAction(selectAllAction);

    // This timer is used to scroll the view while the user is
    // moving the mouse past the top/bottom edge of the widget while selecting.
    selectionTimer = new QTimer(this);
    selectionTimer->setInterval(1000 / 30);
    selectionTimer->setSingleShot(false);
    selectionTimer->start();
    connect(selectionTimer, &QTimer::timeout, this, &ChatWidget::onSelectionTimerTimeout);

    // Background worker
    // Updates the layout of all chat-lines after a resize
    workerTimer = new QTimer(this);
    workerTimer->setSingleShot(false);
    workerTimer->setInterval(5);
    connect(workerTimer, &QTimer::timeout, this, &ChatWidget::onWorkerTimeout);

    // This timer is used to detect multiple clicks
    multiClickTimer = new QTimer(this);
    multiClickTimer->setSingleShot(true);
    multiClickTimer->setInterval(QApplication::doubleClickInterval());
    connect(multiClickTimer, &QTimer::timeout, this, &ChatWidget::onMultiClickTimeout);

    // selection
    connect(this, &ChatWidget::selectionChanged, this, [this]() {
        copyAction->setEnabled(hasTextToBeCopied());
        copySelectedText(true);
    });

    connect(&style, &Style::themeReload, this, &ChatWidget::reloadTheme);

    reloadTheme();
    retranslateUi();
    Translator::registerHandler(std::bind(&ChatWidget::retranslateUi, this), this);

    connect(this, &ChatWidget::renderFinished, this, &ChatWidget::onRenderFinished);
    connect(&chatLog_, &IChatLog::itemUpdated, this, &ChatWidget::onMessageUpdated);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &ChatWidget::onScrollValueChanged);

    auto firstChatLogIdx = clampedAdd(chatLog_.getNextIdx(), -100, chatLog_);
    renderMessages(firstChatLogIdx, chatLog_.getNextIdx());
}

ChatWidget::~ChatWidget()
{
    Translator::unregister(this);

    // Remove chatlines from scene
    for (ChatLine::Ptr l : *chatLineStorage)
        l->removeFromScene();

    if (busyNotification)
        busyNotification->removeFromScene();

    if (typingNotification)
        typingNotification->removeFromScene();
}

void ChatWidget::clearSelection()
{
    if (selectionMode == SelectionMode::None)
        return;

    forEachLineIn(selFirstRow, selLastRow, *chatLineStorage, [&] (ChatLine::Ptr& line) {
        line->selectionCleared();
    });

    selFirstRow.reset();
    selLastRow.reset();
    selClickedCol = -1;
    selClickedRow.reset();

    selectionMode = SelectionMode::None;
    emit selectionChanged();

    updateMultiSelectionRect();
}

QRect ChatWidget::getVisibleRect() const
{
    return mapToScene(viewport()->rect()).boundingRect().toRect();
}

void ChatWidget::updateSceneRect()
{
    setSceneRect(calculateSceneRect());
}

void ChatWidget::layout(int start, int end, qreal width)
{
    if (chatLineStorage->empty())
        return;

    qreal h = 0.0;

    // Line at start-1 is considered to have the correct position. All following lines are
    // positioned in respect to this line.
    if (start - 1 >= 0)
        h = (*chatLineStorage)[start - 1]->sceneBoundingRect().bottom() + lineSpacing;

    start = clamp<int>(start, 0, chatLineStorage->size());
    end = clamp<int>(end + 1, 0, chatLineStorage->size());

    for (int i = start; i < end; ++i) {
        ChatLine* l = (*chatLineStorage)[i].get();

        l->layout(width, QPointF(0.0, h));
        h += l->sceneBoundingRect().height() + lineSpacing;
    }
}

void ChatWidget::mousePressEvent(QMouseEvent* ev)
{
    QGraphicsView::mousePressEvent(ev);

    if (ev->button() == Qt::LeftButton) {
        clickPos = ev->pos();
        clearSelection();
    }

    if (lastClickButton == ev->button()) {
        // Counts only single clicks and first click of doule click
        clickCount++;
    }
    else {
        clickCount = 1; // restarting counter
        lastClickButton = ev->button();
    }
    lastClickPos = ev->pos();

    // Triggers on odd click counts
    handleMultiClickEvent();
}

void ChatWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    QGraphicsView::mouseReleaseEvent(ev);

    selectionScrollDir = AutoScrollDirection::NoDirection;

    multiClickTimer->start();
}

void ChatWidget::mouseMoveEvent(QMouseEvent* ev)
{
    QGraphicsView::mouseMoveEvent(ev);

    QPointF scenePos = mapToScene(ev->pos());

    if (ev->buttons() & Qt::LeftButton) {
        // autoscroll
        if (ev->pos().y() < 0)
            selectionScrollDir = AutoScrollDirection::Up;
        else if (ev->pos().y() > height())
            selectionScrollDir = AutoScrollDirection::Down;
        else
            selectionScrollDir = AutoScrollDirection::NoDirection;

        // select
        if (selectionMode == SelectionMode::None
            && (clickPos - ev->pos()).manhattanLength() > QApplication::startDragDistance()) {
            QPointF sceneClickPos = mapToScene(clickPos.toPoint());
            ChatLine::Ptr line = findLineByPosY(scenePos.y());

            ChatLineContent* content = getContentFromPos(sceneClickPos);
            if (content) {
                selClickedRow = line;
                selClickedCol = content->getColumn();
                selFirstRow = line;
                selLastRow = line;

                content->selectionStarted(sceneClickPos);

                selectionMode = SelectionMode::Precise;

                // ungrab mouse grabber
                if (scene->mouseGrabberItem())
                    scene->mouseGrabberItem()->ungrabMouse();
            } else if (line.get()) {
                selClickedRow = line;
                selFirstRow = selClickedRow;
                selLastRow = selClickedRow;

                selectionMode = SelectionMode::Multi;
            }
        }

        if (selectionMode != SelectionMode::None) {
            ChatLineContent* content = getContentFromPos(scenePos);
            ChatLine::Ptr line = findLineByPosY(scenePos.y());

            if (content) {
                int col = content->getColumn();

                if (line == selClickedRow && col == selClickedCol) {
                    selectionMode = SelectionMode::Precise;

                    content->selectionMouseMove(scenePos);
                    selGraphItem->hide();
                } else if (col != selClickedCol) {
                    selectionMode = SelectionMode::Multi;

                    line->selectionCleared();
                }
            } else if (line.get()) {
                if (line != selClickedRow) {
                    selectionMode = SelectionMode::Multi;
                    line->selectionCleared();
                }
            } else {
                return;
            }

            auto selClickedIt = chatLineStorage->find(selClickedRow);
            auto lineIt = chatLineStorage->find(line);
            if (lineIt > selClickedIt)
                selLastRow = line;

            if (lineIt <= selClickedIt)
                selFirstRow = line;

            updateMultiSelectionRect();
        }

        emit selectionChanged();
    }
}

// Much faster than QGraphicsScene::itemAt()!
ChatLineContent* ChatWidget::getContentFromPos(QPointF scenePos) const
{
    if (chatLineStorage->empty())
        return nullptr;

    auto itr =
        std::lower_bound(chatLineStorage->begin(), chatLineStorage->end(), scenePos.y(), ChatLine::lessThanBSRectBottom);

    // find content
    if (itr != chatLineStorage->end() && (*itr)->sceneBoundingRect().contains(scenePos))
        return (*itr)->getContent(scenePos);

    return nullptr;
}

bool ChatWidget::isOverSelection(QPointF scenePos) const
{
    if (selectionMode == SelectionMode::Precise) {
        ChatLineContent* content = getContentFromPos(scenePos);

        if (content)
            return content->isOverSelection(scenePos);
    } else if (selectionMode == SelectionMode::Multi) {
        if (selGraphItem->rect().contains(scenePos))
            return true;
    }

    return false;
}

qreal ChatWidget::useableWidth() const
{
    return width() - verticalScrollBar()->sizeHint().width() - margins.right() - margins.left();
}

void ChatWidget::insertChatlines(std::map<ChatLogIdx, ChatLine::Ptr> chatLines)
{
    if (chatLines.empty())
        return;

    bool allLinesAtEnd = !chatLineStorage->hasIndexedMessage() || chatLines.begin()->first > chatLineStorage->lastIdx();
    auto startLineSize = chatLineStorage->size();

    QGraphicsScene::ItemIndexMethod oldIndexMeth = scene->itemIndexMethod();
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    for (auto const& chatLine : chatLines) {
        auto idx = chatLine.first;
        auto const& l = chatLine.second;

        chatLineStorage->insertChatMessage(idx, chatLog.at(idx).getTimestamp(), l);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
        auto date = chatLog.at(idx).getTimestamp().date().startOfDay();
#else
        auto date = QDateTime(chatLog.at(idx).getTimestamp().date());
#endif

        if (!chatLineStorage->contains(date)) {
            // If there is no dateline for the given date we need to insert it
            // above the line we'd like to insert.
            auto dateLine = createDateMessage(date, documentCache, settings, style);
            chatLineStorage->insertDateLine(date, dateLine);
            dateLine->addToScene(scene);
            dateLine->visibilityChanged(false);
        }

        l->addToScene(scene);

        // Optimization copied from previous implementation of upwards
        // rendering. This will be changed when we call updateVisibility
        // later
        l->visibilityChanged(false);
    }

    scene->setItemIndexMethod(oldIndexMeth);

    // If all insertions are at the bottom we can get away with only rendering
    // the updated lines, otherwise we need to go through the resize workflow to
    // re-layout everything asynchronously.
    //
    // NOTE: This can make flow from the callers a little frustrating as you
    // have to rely on the onRenderFinished callback to continue doing any work,
    // even if all rendering is done synchronously
    if (allLinesAtEnd) {
        bool stickToBtm = stickToBottom();

        // partial refresh
        layout(startLineSize, chatLineStorage->size(), useableWidth());
        updateSceneRect();

        if (stickToBtm)
            scrollToBottom();

        checkVisibility();
        updateTypingNotification();
        updateMultiSelectionRect();

        emit renderFinished();
    } else {
        startResizeWorker();
    }
}

bool ChatWidget::stickToBottom() const
{
    return verticalScrollBar()->value() == verticalScrollBar()->maximum();
}

void ChatWidget::scrollToBottom()
{
    updateSceneRect();
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void ChatWidget::startResizeWorker()
{
    if (chatLineStorage->empty())
        return;

    // (re)start the worker
    if (!workerTimer->isActive()) {
        // these values must not be reevaluated while the worker is running
        workerStb = stickToBottom();

        if (!visibleLines.empty())
            workerAnchorLine = visibleLines.first();
    }

    // switch to busy scene displaying the busy notification if there is a lot
    // of text to be resized
    int txt = 0;
    for (ChatLine::Ptr line : *chatLineStorage) {
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
}

void ChatWidget::mouseDoubleClickEvent(QMouseEvent* ev)
{
    QPointF scenePos = mapToScene(ev->pos());
    ChatLineContent* content = getContentFromPos(scenePos);
    ChatLine::Ptr line = findLineByPosY(scenePos.y());

    if (content) {
        content->selectionDoubleClick(scenePos);
        selClickedCol = content->getColumn();
        selClickedRow = line;
        selFirstRow = line;
        selLastRow = line;
        selectionMode = SelectionMode::Precise;

        emit selectionChanged();
    }

    if (lastClickButton == ev->button()) {
        // Counts the second click of double click
        clickCount++;
    }
    else {
        clickCount = 1; // restarting counter
        lastClickButton = ev->button();
    }
    lastClickPos = ev->pos();

    // Triggers on even click counts
    handleMultiClickEvent();
}

QString ChatWidget::getSelectedText() const
{
    if (selectionMode == SelectionMode::Precise) {
        return selClickedRow->content[selClickedCol]->getSelectedText();
    } else if (selectionMode == SelectionMode::Multi) {
        // build a nicely formatted message
        QString out;

        forEachLineIn(selFirstRow, selLastRow, *chatLineStorage, [&] (ChatLine::Ptr& line) {
            if (line->content[1]->getText().isEmpty())
                return;

            QString timestamp = line->content[2]->getText().isEmpty()
                                    ? tr("pending")
                                    : line->content[2]->getText();
            QString author = line->content[0]->getText();
            QString msg = line->content[1]->getText();

            out +=
                QString(out.isEmpty() ? QStringLiteral("[%2] %1: %3") : QStringLiteral("\n[%2] %1: %3")).arg(author, timestamp, msg);
        });

        return out;
    }

    return QString();
}

bool ChatWidget::isEmpty() const
{
    return chatLineStorage->empty();
}

bool ChatWidget::hasTextToBeCopied() const
{
    return selectionMode != SelectionMode::None;
}

/**
 * @brief Finds the chat line object at a position on screen
 * @param pos Position on screen in global coordinates
 * @sa getContentFromPos()
 */
ChatLineContent* ChatWidget::getContentFromGlobalPos(QPoint pos) const
{
    return getContentFromPos(mapToScene(mapFromGlobal(pos)));
}

void ChatWidget::clear()
{
    clearSelection();

    QVector<ChatLine::Ptr> savedLines;

    for (auto it = chatLineStorage->begin(); it != chatLineStorage->end();) {
        if (!isActiveFileTransfer(*it)) {
            (*it)->removeFromScene();
            it = chatLineStorage->erase(it);
        } else {
            it++;
        }
    }

    visibleLines.clear();

    checkVisibility();
    updateSceneRect();
}

void ChatWidget::copySelectedText(bool toSelectionBuffer) const
{
    QString text = getSelectedText();
    QClipboard* clipboard = QApplication::clipboard();

    if (clipboard && !text.isNull())
        clipboard->setText(text, toSelectionBuffer ? QClipboard::Selection : QClipboard::Clipboard);
}

void ChatWidget::setTypingNotificationVisible(bool visible)
{
    if (typingNotification.get()) {
        typingNotification->setVisible(visible);
        updateTypingNotification();
    }
}

void ChatWidget::setTypingNotificationName(const QString& displayName)
{
    if (!typingNotification.get()) {
        setTypingNotification();
    }

    Text* text = static_cast<Text*>(typingNotification->getContent(1));
    QString typingDiv = "<div class=typing>%1</div>";
    text->setText(typingDiv.arg(tr("%1 is typing").arg(displayName)));

    updateTypingNotification();
}

void ChatWidget::scrollToLine(ChatLine::Ptr line)
{
    if (!line.get())
        return;

    updateSceneRect();
    verticalScrollBar()->setValue(line->sceneBoundingRect().top());
}

void ChatWidget::selectAll()
{
    if (chatLineStorage->empty())
        return;

    clearSelection();

    selectionMode = SelectionMode::Multi;
    selFirstRow = chatLineStorage->front();;
    selLastRow = chatLineStorage->back();

    emit selectionChanged();
    updateMultiSelectionRect();
}

void ChatWidget::fontChanged(const QFont& font)
{
    for (ChatLine::Ptr l : *chatLineStorage) {
        l->fontChanged(font);
    }
}

void ChatWidget::reloadTheme()
{
    setStyleSheet(style.getStylesheet("chatArea/chatArea.css", settings));
    setBackgroundBrush(QBrush(style.getColor(Style::ColorPalette::GroundBase), Qt::SolidPattern));
    selectionRectColor = style.getColor(Style::ColorPalette::SelectText);
    selGraphItem->setBrush(QBrush(selectionRectColor));
    selGraphItem->setPen(QPen(selectionRectColor.darker(120)));
    setTypingNotification();

    for (ChatLine::Ptr l : *chatLineStorage) {
        l->reloadTheme();
    }
}

void ChatWidget::startSearch(const QString& phrase, const ParameterSearch& parameter)
{
    disableSearchText();

    bool bForwardSearch = false;
    switch (parameter.period) {
    case PeriodSearch::WithTheFirst: {
        bForwardSearch = true;
        searchPos.logIdx = chatLog.getFirstIdx();
        searchPos.numMatches = 0;
        break;
    }
    case PeriodSearch::WithTheEnd:
    case PeriodSearch::None: {
        bForwardSearch = false;
        searchPos.logIdx = chatLog.getNextIdx();
        searchPos.numMatches = 0;
        break;
    }
    case PeriodSearch::AfterDate: {
        bForwardSearch = true;
        searchPos.logIdx = firstItemAfterDate(parameter.date, chatLog);
        searchPos.numMatches = 0;
        break;
    }
    case PeriodSearch::BeforeDate: {
        bForwardSearch = false;
        searchPos.logIdx = firstItemAfterDate(parameter.date, chatLog);
        searchPos.numMatches = 0;
        break;
    }
    }

    if (bForwardSearch) {
        onSearchDown(phrase, parameter);
    } else {
        onSearchUp(phrase, parameter);
    }
}

void ChatWidget::onSearchUp(const QString& phrase, const ParameterSearch& parameter)
{
    auto result = chatLog.searchBackward(searchPos, phrase, parameter);
    handleSearchResult(result, SearchDirection::Up);
}

void ChatWidget::onSearchDown(const QString& phrase, const ParameterSearch& parameter)
{
    auto result = chatLog.searchForward(searchPos, phrase, parameter);
    handleSearchResult(result, SearchDirection::Down);
}

void ChatWidget::handleSearchResult(SearchResult result, SearchDirection direction)
{
    if (!result.found) {
        emit messageNotFoundShow(direction);
        return;
    }

    disableSearchText();

    searchPos = result.pos;

    auto selectText = [this, result] {
        // With fast changes our callback could become invalid, ensure that the
        // index we want to view is still actually visible
        if (!chatLineStorage->contains(searchPos.logIdx))
            return;

        auto msg = (*chatLineStorage)[searchPos.logIdx];
        scrollToLine(msg);

        auto text = qobject_cast<Text*>(msg->getContent(1));
        text->selectText(result.exp, std::make_pair(result.start, result.len));
    };

    // If the requested element is visible the render completion callback will
    // not be called, we need to figure out which path we're going to take
    // before we take it.
    if (chatLineStorage->contains(searchPos.logIdx)) {
        jumpToIdx(searchPos.logIdx);
        selectText();
    } else {
        renderCompletionFns.push_back(selectText);
        jumpToIdx(searchPos.logIdx);
    }

}

void ChatWidget::forceRelayout()
{
    startResizeWorker();
}

void ChatWidget::checkVisibility()
{
    if (chatLineStorage->empty())
        return;

    // find first visible line
    auto lowerBound = std::lower_bound(chatLineStorage->begin(), chatLineStorage->end(), getVisibleRect().top(),
                                       ChatLine::lessThanBSRectBottom);

    // find last visible line
    auto upperBound = std::lower_bound(lowerBound, chatLineStorage->end(), getVisibleRect().bottom(),
                                       ChatLine::lessThanBSRectTop);

    const ChatLine::Ptr lastLineBeforeVisible = lowerBound == chatLineStorage->begin()
        ? ChatLine::Ptr()
        : *std::prev(lowerBound);

    // set visibilty
    QList<ChatLine::Ptr> newVisibleLines;
    for (auto itr = lowerBound; itr != upperBound; ++itr) {
        newVisibleLines.append(*itr);

        if (!visibleLines.contains(*itr))
            (*itr)->visibilityChanged(true);

        visibleLines.removeOne(*itr);
    }

    // these lines are no longer visible
    for (ChatLine::Ptr line : visibleLines)
        line->visibilityChanged(false);

    visibleLines = newVisibleLines;

    if (!visibleLines.isEmpty()) {
        emit firstVisibleLineChanged(lastLineBeforeVisible, visibleLines.at(0));
    }
}

void ChatWidget::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    checkVisibility();
}

void ChatWidget::resizeEvent(QResizeEvent* ev)
{
    bool stb = stickToBottom();

    if (ev->size().width() != ev->oldSize().width()) {
        startResizeWorker();
        stb = false; // let the resize worker handle it
    }

    QGraphicsView::resizeEvent(ev);

    if (stb)
        scrollToBottom();

    updateBusyNotification();
}

void ChatWidget::updateMultiSelectionRect()
{
    if (selectionMode == SelectionMode::Multi && selFirstRow && selLastRow) {
        QRectF selBBox;
        selBBox = selBBox.united(selFirstRow->sceneBoundingRect());
        selBBox = selBBox.united(selLastRow->sceneBoundingRect());

        if (selGraphItem->rect() != selBBox)
            scene->invalidate(selGraphItem->rect());

        selGraphItem->setRect(selBBox);
        selGraphItem->show();
    } else {
        selGraphItem->hide();
    }
}

void ChatWidget::updateTypingNotification()
{
    ChatLine* notification = typingNotification.get();
    if (!notification)
        return;

    qreal posY = 0.0;

    if (!chatLineStorage->empty())
        posY = chatLineStorage->back()->sceneBoundingRect().bottom() + lineSpacing;

    notification->layout(useableWidth(), QPointF(0.0, posY));
}

void ChatWidget::updateBusyNotification()
{
    // repoisition the busy notification (centered)
    busyNotification->layout(useableWidth(), getVisibleRect().topLeft()
                                                    + QPointF(0, getVisibleRect().height() / 2.0));
}

ChatLine::Ptr ChatWidget::findLineByPosY(qreal yPos) const
{
    auto itr = std::lower_bound(chatLineStorage->begin(), chatLineStorage->end(), yPos, ChatLine::lessThanBSRectBottom);

    if (itr != chatLineStorage->end())
        return *itr;

    return ChatLine::Ptr();
}

void ChatWidget::removeLines(ChatLogIdx begin, ChatLogIdx end)
{
    if (!chatLineStorage->hasIndexedMessage()) {
        // No indexed lines to remove
        return;
    }

    begin = clamp<ChatLogIdx>(begin, chatLineStorage->firstIdx(), chatLineStorage->lastIdx());
    end = clamp<ChatLogIdx>(end, chatLineStorage->firstIdx(), chatLineStorage->lastIdx()) + 1;

    // NOTE: Optimization potential if this find proves to be too expensive.
    // Batching all our erases into one call would be more efficient
    for (auto it = chatLineStorage->find(begin); it != chatLineStorage->find(end);) {
        (*it)->removeFromScene();
        it = chatLineStorage->erase(it);
    }

    // We need to re-layout anything that is after any line we removed. We could
    // probably be smarter and try to only re-render anything under what we
    // removed, but with the sliding window there doesn't seem to be much need
    if (chatLineStorage->hasIndexedMessage() && begin <= chatLineStorage->lastIdx()) {
        layout(0, chatLineStorage->size(), useableWidth());
    }
}

QRectF ChatWidget::calculateSceneRect() const
{
    qreal bottom = (chatLineStorage->empty() ? 0.0 : chatLineStorage->back()->sceneBoundingRect().bottom());

    if (typingNotification.get() != nullptr)
        bottom += typingNotification->sceneBoundingRect().height() + lineSpacing;

    return QRectF(-margins.left(), -margins.top(), useableWidth(),
                  bottom + margins.bottom() + margins.top());
}

void ChatWidget::onSelectionTimerTimeout()
{
    const int scrollSpeed = 10;

    switch (selectionScrollDir) {
    case AutoScrollDirection::Up:
        verticalScrollBar()->setValue(verticalScrollBar()->value() - scrollSpeed);
        break;
    case AutoScrollDirection::Down:
        verticalScrollBar()->setValue(verticalScrollBar()->value() + scrollSpeed);
        break;
    default:
        break;
    }
}

void ChatWidget::onWorkerTimeout()
{
    // Fairly arbitrary but
    // large values will make the UI unresponsive
    const int stepSize = 50;

    layout(workerLastIndex, workerLastIndex + stepSize, useableWidth());
    workerLastIndex += stepSize;

    // done?
    if (workerLastIndex >= chatLineStorage->size()) {
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

        emit renderFinished();
    }
}

void ChatWidget::onMultiClickTimeout()
{
    clickCount = 0;
}

void ChatWidget::onMessageUpdated(ChatLogIdx idx)
{
    if (shouldRenderMessage(idx)) {
        renderMessage(idx);
    }
}

void ChatWidget::renderMessage(ChatLogIdx idx)
{
    renderMessages(idx, idx + 1);
}

void ChatWidget::renderMessages(ChatLogIdx begin, ChatLogIdx end)
{
    auto linesToRender = std::map<ChatLogIdx, ChatLine::Ptr>();

    for (auto i = begin; i < end; ++i) {
        bool alreadyRendered = chatLineStorage->contains(i);
        bool prevIdxRendered = i != begin || chatLineStorage->contains(i - 1);

        auto chatMessage = alreadyRendered ? (*chatLineStorage)[i] : ChatLine::Ptr();
        renderItem(chatLog.at(i), needsToHideName(i, prevIdxRendered), colorizeNames, chatMessage);

        if (!alreadyRendered) {
            linesToRender.insert({i, chatMessage});
        }
    }

    insertChatlines(linesToRender);
}

void ChatWidget::setRenderedWindowStart(ChatLogIdx begin)
{
    // End of the window is pre-determined as a hardcoded window size relative
    // to the start
    auto end = clampedAdd(begin, maxWindowSize, chatLog);

    // Use invalid + equal ChatLogIdx to force a full re-render if we do not
    // have an indexed message to compare to
    ChatLogIdx currentStart = ChatLogIdx(-1);
    ChatLogIdx currentEnd = ChatLogIdx(-1);

    if (chatLineStorage->hasIndexedMessage()) {
        currentStart = chatLineStorage->firstIdx();
        currentEnd = chatLineStorage->lastIdx() + 1;
    }

    // If the window is already where we have no work to do
    if (currentStart == begin) {
        emit renderFinished();
        return;
    }

    // NOTE: This is more than an optimization, this is important for
    // selection consistency. If we re-create lines that are already rendered
    // the selXXXRow members will now be pointing to the wrong ChatLine::Ptr!
    // Please be sure to test selection logic when scrolling around loading
    // boundaries if changing this logic.
    if (begin < currentEnd && begin > currentStart) {
        // Remove leading lines
        removeLines(currentStart, begin);
        renderMessages(currentEnd, end);
    }
    else if (end <= currentEnd && end > currentStart) {
        // Remove trailing lines
        removeLines(end, currentEnd);
        renderMessages(begin, currentStart);
    }
    else {
        removeLines(currentStart, currentEnd);
        renderMessages(begin, end);
    }
}

void ChatWidget::setRenderedWindowEnd(ChatLogIdx end)
{
    // Off by 1 since the maxWindowSize is not inclusive
    auto start = clampedAdd(end, -maxWindowSize + 1, chatLog);

    setRenderedWindowStart(start);
}

void ChatWidget::onRenderFinished()
{
    // We have to back these up before we run them, because people might queue
    // on _more_ items on top of the ones we want. If they do this while we're
    // iterating we can hit some memory corruption issues
    auto renderCompletionFnsLocal = renderCompletionFns;
    renderCompletionFns.clear();

    while (renderCompletionFnsLocal.size()) {
        renderCompletionFnsLocal.back()();
        renderCompletionFnsLocal.pop_back();
    }

    // NOTE: this is a regression from previous behavior. We used to be able to
    // load an infinite amount of chat and copy paste it out. Now we limit the
    // user to 300 elements and any time the elements change our selection gets
    // invalidated. This could be improved in the future but for now I  do not
    // believe this is a serious usage impediment. Chats can be exported if a
    // user really needs more than 300 messages to be copied
    if (chatLineStorage->find(selFirstRow) == chatLineStorage->end() ||
        chatLineStorage->find(selLastRow) == chatLineStorage->end() ||
        chatLineStorage->find(selClickedRow) == chatLineStorage->end())
    {
        // FIXME: Segfault when selecting while scrolling down
        clearSelection();
    }
}

void ChatWidget::onScrollValueChanged(int value)
{
    if (!chatLineStorage->hasIndexedMessage()) {
        // This could be a little better. On a cleared screen we should probably
        // be able to scroll, but this makes the rest of this function easier
        return;
    }

    // When we hit the end of our scroll bar we change the content that's in the
    // viewport. In this process our scroll value may end up changing again! We
    // avoid this by changing our scroll position to match where it was before
    // started our viewport change, but we need to filter out any intermediate
    // scroll events triggered before we get to that point
    if (!scrollMonitoringEnabled) {
        return;
    }

    if (value == verticalScrollBar()->minimum())
    {
        auto idx = clampedAdd(chatLineStorage->firstIdx(), -static_cast<int>(windowChunkSize), chatLog);

        if (idx != chatLineStorage->firstIdx()) {
            auto currentTop = (*chatLineStorage)[chatLineStorage->firstIdx()];

            renderCompletionFns.push_back([this, currentTop] {
                scrollToLine(currentTop);
                scrollMonitoringEnabled = true;
            });

            scrollMonitoringEnabled = false;
            setRenderedWindowStart(idx);
        }

    }
    else if (value == verticalScrollBar()->maximum())
    {
        auto idx = clampedAdd(chatLineStorage->lastIdx(), static_cast<int>(windowChunkSize), chatLog);

        if (idx != chatLineStorage->lastIdx() + 1) {
            // FIXME: This should be the top line
            auto currentBottomIdx = chatLineStorage->lastIdx();
            auto currentTopPx = mapToScene(0, 0).y();
            auto currentBottomPx = (*chatLineStorage)[currentBottomIdx]->sceneBoundingRect().bottom();
            auto bottomOffset = currentBottomPx - currentTopPx;

            renderCompletionFns.push_back([this, currentBottomIdx, bottomOffset] {
                auto it = chatLineStorage->find(currentBottomIdx);
                if (it != chatLineStorage->end()) {
                    updateSceneRect();
                    verticalScrollBar()->setValue((*it)->sceneBoundingRect().bottom() - bottomOffset);
                    scrollMonitoringEnabled = true;
                }
            });

            scrollMonitoringEnabled = false;
            setRenderedWindowEnd(idx);
        }
    }
}

void ChatWidget::handleMultiClickEvent()
{
    // Ignore single or double clicks
    if (clickCount < 2)
        return;

    switch (clickCount) {
    case 3:
        QPointF scenePos = mapToScene(lastClickPos);
        ChatLineContent* content = getContentFromPos(scenePos);
        ChatLine::Ptr line = findLineByPosY(scenePos.y());

        if (content) {
            content->selectionTripleClick(scenePos);
            selClickedCol = content->getColumn();
            selClickedRow = line;
            selFirstRow = line;
            selLastRow = line;
            selectionMode = SelectionMode::Precise;

            emit selectionChanged();
        }
        break;
    }
}

void ChatWidget::showEvent(QShowEvent* event)
{
    std::ignore = event;
    // Empty.
    // The default implementation calls centerOn - for some reason - causing
    // the scrollbar to move.
}

void ChatWidget::hideEvent(QHideEvent* event)
{
    std::ignore = event;
    // Purge accumulated lines from the chatlog. We do not purge messages while
    // the chatlog is open because it causes flickers. When a user leaves the
    // chat we take the opportunity to remove old messages. If a user only has
    // one friend this could end up accumulating chat logs until they restart
    // qTox, but that isn't a regression from previously released behavior.

    auto numLinesToRemove = chatLineStorage->size() > maxWindowSize
        ? chatLineStorage->size() - maxWindowSize
        : 0;

    if (numLinesToRemove > 0) {
        removeLines(chatLineStorage->firstIdx(), chatLineStorage->firstIdx() + numLinesToRemove);
        startResizeWorker();
    }
}

void ChatWidget::focusInEvent(QFocusEvent* ev)
{
    QGraphicsView::focusInEvent(ev);

    if (selectionMode != SelectionMode::None) {
        selGraphItem->setBrush(QBrush(selectionRectColor));

        auto endIt = chatLineStorage->find(selLastRow);
        // Increase by one since this selLastRow is inclusive, not exclusive
        // like our loop expects
        if (endIt != chatLineStorage->end()) {
            endIt++;
        }

        for (auto it = chatLineStorage->begin(); it != chatLineStorage->end() && it != endIt; ++it)
            (*it)->selectionFocusChanged(true);
    }
}

void ChatWidget::focusOutEvent(QFocusEvent* ev)
{
    QGraphicsView::focusOutEvent(ev);

    if (selectionMode != SelectionMode::None) {
        selGraphItem->setBrush(QBrush(selectionRectColor.lighter(120)));

        auto endIt = chatLineStorage->find(selLastRow);
        // Increase by one since this selLastRow is inclusive, not exclusive
        // like our loop expects
        if (endIt != chatLineStorage->end()) {
            endIt++;
        }

        for (auto it = chatLineStorage->begin(); it != chatLineStorage->end() && it != endIt; ++it)
            (*it)->selectionFocusChanged(false);
    }
}

void ChatWidget::wheelEvent(QWheelEvent *event)
{
    QGraphicsView::wheelEvent(event);
    checkVisibility();
}

void ChatWidget::retranslateUi()
{
    copyAction->setText(tr("Copy"));
    selectAllAction->setText(tr("Select all"));
}

bool ChatWidget::isActiveFileTransfer(ChatLine::Ptr l)
{
    int count = l->getColumnCount();
    for (int i = 0; i < count; ++i) {
        ChatLineContent* content = l->getContent(i);
        ChatLineContentProxy* proxy = qobject_cast<ChatLineContentProxy*>(content);
        if (!proxy)
            continue;

        QWidget* widget = proxy->getWidget();
        FileTransferWidget* transferWidget = qobject_cast<FileTransferWidget*>(widget);
        if (transferWidget && transferWidget->isActive())
            return true;
    }

    return false;
}

void ChatWidget::setTypingNotification()
{
    typingNotification = ChatMessage::createTypingNotification(documentCache, settings, style);
    typingNotification->visibilityChanged(true);
    typingNotification->setVisible(false);
    typingNotification->addToScene(scene);
    updateTypingNotification();
}


void ChatWidget::renderItem(const ChatLogItem& item, bool hideName, bool colorizeNames_, ChatLine::Ptr& chatMessage)
{
    const auto& sender = item.getSender();

    bool isSelf = sender == core.getSelfPublicKey();

    switch (item.getContentType()) {
    case ChatLogItem::ContentType::message: {
        const auto& chatLogMessage = item.getContentAsMessage();

        renderMessageRaw(item.getDisplayName(), isSelf, colorizeNames_, chatLogMessage,
            chatMessage, documentCache, smileyPack, settings, style);

        break;
    }
    case ChatLogItem::ContentType::fileTransfer: {
        const auto& file = item.getContentAsFile();
        renderFile(item.getDisplayName(), file.file, isSelf, item.getTimestamp(), chatMessage);
        break;
    }
    case ChatLogItem::ContentType::systemMessage: {
        const auto& systemMessage = item.getContentAsSystemMessage();

        auto chatMessageType = getChatMessageType(systemMessage);
        chatMessage = ChatMessage::createChatInfoMessage(systemMessage.toString(),
            chatMessageType, systemMessage.timestamp, documentCache, settings,
            style);
        // Ignore caller's decision to hide the name. We show the icon in the
        // slot of the sender's name so we always want it visible
        hideName = false;
        break;
    }
    }

    if (hideName) {
        chatMessage->getContent(0)->hide();
    }
}

void ChatWidget::renderFile(QString displayName, ToxFile file, bool isSelf, QDateTime timestamp,
                ChatLine::Ptr& chatMessage)
{
    if (!chatMessage) {
        CoreFile* coreFile = core.getCoreFile();
        assert(coreFile);
        chatMessage = ChatMessage::createFileTransferMessage(displayName, *coreFile,
            file, isSelf, timestamp, documentCache, settings, style, messageBoxManager);
    } else {
        auto proxy = static_cast<ChatLineContentProxy*>(chatMessage->getContent(1));
        assert(proxy->getWidgetType() == ChatLineContentProxy::FileTransferWidgetType);
        auto ftWidget = static_cast<FileTransferWidget*>(proxy->getWidget());
        ftWidget->onFileTransferUpdate(file);
    }
}

/**
 * @brief Determine if the name at the given idx needs to be hidden
 * @param idx ChatLogIdx of the message
 * @param prevIdxRendered Hint if the previous index is going to be rendered at
 * all. If the previous line is not rendered we always show the name
 * @return True if the name should be hidden, false otherwise
 */
bool ChatWidget::needsToHideName(ChatLogIdx idx, bool prevIdxRendered) const
{
    // If the previous message is not rendered we should show the name
    // regardless of other constraints

    if (!prevIdxRendered) {
        return false;
    }

    const auto& prevItem = chatLog.at(idx - 1);
    const auto& currentItem = chatLog.at(idx);

    // Always show the * in the name field for action messages
    if (currentItem.getContentType() == ChatLogItem::ContentType::message
        && currentItem.getContentAsMessage().message.isAction) {
        return false;
    }

    qint64 messagesTimeDiff = prevItem.getTimestamp().secsTo(currentItem.getTimestamp());
    return currentItem.getSender() == prevItem.getSender()
           && messagesTimeDiff < repNameAfter;

    return false;
}

bool ChatWidget::shouldRenderMessage(ChatLogIdx idx) const
{
    return chatLineStorage->contains(idx) ||
        (
            chatLineStorage->contains(idx - 1) && idx + 1 == chatLog.getNextIdx()
        ) || chatLineStorage->empty();
}

void ChatWidget::disableSearchText()
{
    if (!chatLineStorage->contains(searchPos.logIdx)) {
        return;
    }

    auto line = (*chatLineStorage)[searchPos.logIdx];
    auto text = qobject_cast<Text*>(line->getContent(1));
    text->deselectText();
}

void ChatWidget::removeSearchPhrase()
{
    disableSearchText();
}

void ChatWidget::jumpToDate(QDate date) {
    auto idx = firstItemAfterDate(date, chatLog);
    jumpToIdx(idx);
}

void ChatWidget::jumpToIdx(ChatLogIdx idx)
{
    if (idx == chatLog.getNextIdx()) {
        idx = chatLog.getNextIdx() - 1;
    }

    if (chatLineStorage->contains(idx)) {
        scrollToLine((*chatLineStorage)[idx]);
        return;
    }

    // If the requested idx is not currently rendered we need to request a
    // render and jump to the requested line after the render completes
    renderCompletionFns.push_back([this, idx] {
        if (chatLineStorage->contains(idx)) {
            scrollToLine((*chatLineStorage)[idx]);
        }
    });

    // If the chatlog is empty it's likely the user has just cleared. In this
    // case it makes more sense to present the jump as if we're coming from the
    // bottom
    if (chatLineStorage->hasIndexedMessage() && idx > chatLineStorage->lastIdx()) {
        setRenderedWindowEnd(clampedAdd(idx, windowChunkSize, chatLog));
    }
    else {
        setRenderedWindowStart(clampedAdd(idx, -windowChunkSize, chatLog));
    }
}
