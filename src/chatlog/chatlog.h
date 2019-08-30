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

#ifndef CHATLOG_H
#define CHATLOG_H

#include <QDateTime>
#include <QGraphicsView>
#include <QMargins>

#include "chatline.h"
#include "chatmessage.h"
#include "src/widget/style.h"

class QGraphicsScene;
class QGraphicsRectItem;
class QMouseEvent;
class QTimer;
class ChatLineContent;
struct ToxFile;

static const auto DEF_NUM_MSG_TO_LOAD = 100;

class ChatLog : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ChatLog(const bool canRemove, QWidget* parent = nullptr);
    virtual ~ChatLog();

    void insertChatlineAtBottom(ChatLine::Ptr l);
    void insertChatlineAtBottom(const QList<ChatLine::Ptr>& newLines);
    void insertChatlineOnTop(ChatLine::Ptr l);
    void insertChatlinesOnTop(const QList<ChatLine::Ptr>& newLines);
    void clearSelection();
    void clear();
    void copySelectedText(bool toSelectionBuffer = false) const;
    void setBusyNotification(ChatLine::Ptr notification);
    void setTypingNotification(ChatLine::Ptr notification);
    void setTypingNotificationVisible(bool visible);
    void scrollToLine(ChatLine::Ptr line);
    void selectAll();
    void fontChanged(const QFont& font);
    void reloadTheme();
    void moveSelectionRectUpIfMulti(int offset);
    void moveSelectionRectDownIfMulti(int offset);
    void removeFirsts(const int num);
    void removeLasts(const int num);
    void setScroll(const bool scroll);
    int getNumRemove() const;

    QString getSelectedText() const;

    bool isEmpty() const;
    bool hasTextToBeCopied() const;

    ChatLine::Ptr getTypingNotification() const;
    QVector<ChatLine::Ptr> getLines();
    ChatLine::Ptr getLatestLine() const;
    ChatLine::Ptr getFirstLine() const;
    ChatLineContent* getContentFromGlobalPos(QPoint pos) const;
    const uint repNameAfter = 5 * 60;

signals:
    void selectionChanged();
    void workerTimeoutFinished();
    void firstVisibleLineChanged(const ChatLine::Ptr&);
    void loadHistoryLower();
    void loadHistoryUpper();

public slots:
    void forceRelayout();

private slots:
    void onSelectionTimerTimeout();
    void onWorkerTimeout();
    void onMultiClickTimeout();

protected:
    QRectF calculateSceneRect() const;
    QRect getVisibleRect() const;
    ChatLineContent* getContentFromPos(QPointF scenePos) const;

    void layout(int start, int end, qreal width);
    bool isOverSelection(QPointF scenePos) const;
    bool stickToBottom() const;

    qreal useableWidth() const;

    void reposition(int start, int end, qreal deltaY);
    void updateSceneRect();
    void checkVisibility(bool causedWheelEvent = false);
    void scrollToBottom();
    void startResizeWorker(ChatLine::Ptr anchorLine = nullptr);

    virtual void mouseDoubleClickEvent(QMouseEvent* ev) final override;
    virtual void mousePressEvent(QMouseEvent* ev) final override;
    virtual void mouseReleaseEvent(QMouseEvent* ev) final override;
    virtual void mouseMoveEvent(QMouseEvent* ev) final override;
    virtual void scrollContentsBy(int dx, int dy) final override;
    virtual void resizeEvent(QResizeEvent* ev) final override;
    virtual void showEvent(QShowEvent*) final override;
    virtual void focusInEvent(QFocusEvent* ev) final override;
    virtual void focusOutEvent(QFocusEvent* ev) final override;
    virtual void wheelEvent(QWheelEvent *event) final override;

    void updateMultiSelectionRect();
    void updateTypingNotification();
    void updateBusyNotification();

    ChatLine::Ptr findLineByPosY(qreal yPos) const;

private:
    void retranslateUi();
    bool isActiveFileTransfer(ChatLine::Ptr l);
    void handleMultiClickEvent();

private:
    enum SelectionMode
    {
        None,
        Precise,
        Multi,
    };

    enum AutoScrollDirection
    {
        NoDirection,
        Up,
        Down,
    };

    QAction* copyAction = nullptr;
    QAction* selectAllAction = nullptr;
    QGraphicsScene* scene = nullptr;
    QGraphicsScene* busyScene = nullptr;
    QVector<ChatLine::Ptr> lines;
    QList<ChatLine::Ptr> visibleLines;
    ChatLine::Ptr typingNotification;
    ChatLine::Ptr busyNotification;

    // selection
    int selClickedRow = -1; // These 4 are only valid while selectionMode != None
    int selClickedCol = -1;
    int selFirstRow = -1;
    int selLastRow = -1;
    QColor selectionRectColor = Style::getColor(Style::SelectText);
    SelectionMode selectionMode = None;
    QPointF clickPos;
    QGraphicsRectItem* selGraphItem = nullptr;
    QTimer* selectionTimer = nullptr;
    QTimer* workerTimer = nullptr;
    QTimer* multiClickTimer = nullptr;
    AutoScrollDirection selectionScrollDir = NoDirection;
    int clickCount = 0;
    QPoint lastClickPos;
    Qt::MouseButton lastClickButton;
    bool isScroll{true};

    // worker vars
    int workerLastIndex = 0;
    bool workerStb = false;
    ChatLine::Ptr workerAnchorLine;

    // layout
    QMargins margins = QMargins(10, 10, 10, 10);
    qreal lineSpacing = 5.0f;

    int numRemove{0};
    const int maxMessages{300};
    bool canRemove;
};

#endif // CHATLOG_H
