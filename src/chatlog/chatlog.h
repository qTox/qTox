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

#ifndef CHATLOG_H
#define CHATLOG_H

#include <QGraphicsView>
#include <QDateTime>
#include <QMargins>

#include "chatline.h"
#include "chatmessage.h"

class QGraphicsScene;
class QGraphicsRectItem;
class QMouseEvent;
class QTimer;
class ChatLineContent;
struct ToxFile;

class ChatLog : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ChatLog(QWidget* parent = 0);
    virtual ~ChatLog();

    void setVerticalScrollBar(QScrollBar* scrollbar);
    void insertChatlineAtBottom(ChatLine::Ptr l);
    void insertChatlineOnTop(ChatLine::Ptr l);
    void insertChatlineOnTop(const QList<ChatLine::Ptr>& newLines);
    void clearSelection();
    void clear();
    void copySelectedText(bool toSelectionBuffer = false) const;
    void setBusyNotification(ChatLine::Ptr notification);
    void setTypingNotification(ChatLine::Ptr notification);
    void setTypingNotificationVisible(bool visible);
    void scrollToLine(ChatLine::Ptr line);
    void selectAll();
    void forceRelayout();

    QString getSelectedText() const;
    int findText(const QString &text, Qt::CaseSensitivity sensitivity, int &index);
    int findNext(const QString& text, int to, int total, Qt::CaseSensitivity sensitivity);
    int findPrevious(const QString& text, int to, int total, Qt::CaseSensitivity sensitivity);
    const QHash<int, ChatLine::Ptr>& getFoundLines() const;

    bool isEmpty() const;
    bool hasTextToBeCopied() const;
    void addDateMessage(QDate date, ChatMessage::Ptr message);

    ChatLine::Ptr getTypingNotification() const;
    QVector<ChatLine::Ptr> getLines();
    ChatLine::Ptr getLatestLine() const;
    // repetition interval sender name (sec)
    const uint repNameAfter = 5*60;

signals:
    void selectionChanged();

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
    void checkVisibility();
    void scrollToBottom();
    void startResizeWorker();

    virtual void mouseDoubleClickEvent(QMouseEvent* ev) final override;
    virtual void mousePressEvent(QMouseEvent* ev) final override;
    virtual void mouseReleaseEvent(QMouseEvent* ev) final override;
    virtual void mouseMoveEvent(QMouseEvent* ev) final override;
    virtual void scrollContentsBy(int dx, int dy) final override;
    virtual void resizeEvent(QResizeEvent* ev) final override;
    virtual void showEvent(QShowEvent*) final override;
    virtual void focusInEvent(QFocusEvent* ev) final override;
    virtual void focusOutEvent(QFocusEvent* ev) final override;

    void updateMultiSelectionRect();
    void updateTypingNotification();
    void updateBusyNotification();

    ChatLine::Ptr findLineByPosY(qreal yPos) const;

private slots:
    void onSelectionTimerTimeout();
    void onWorkerTimeout();
    void onScrollBarChanged(int value);

private:
    void retranslateUi();

private:
    void updateLayout(int currentWidth, int previousWidth);

    enum SelectionMode {
        None          = 0x00,
        Precise       = 0x01,
        Multi         = 0x02,
        Selected      = Precise | Multi,
        SplitterLeft  = 0x04,
        SplitterRight = 0x08,
        Splitter      = SplitterLeft | SplitterRight,
    };

    enum AutoScrollDirection {
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
    int selClickedRow = -1; //These 4 are only valid while selectionMode != None
    int selClickedCol = -1;
    int selFirstRow = -1;
    int selLastRow = -1;
    QColor selectionRectColor = QColor::fromRgbF(0.23, 0.68, 0.91).lighter(150);
    SelectionMode selectionMode = None;
    QPointF clickPos;
    QGraphicsRectItem* selGraphItem = nullptr;
    QTimer* selectionTimer = nullptr;
    QTimer* workerTimer = nullptr;
    AutoScrollDirection selectionScrollDir = NoDirection;
    int splitterVal;

    //worker vars
    int workerLastIndex = 0;
    bool workerStb = false;
    ChatLine::Ptr workerAnchorLine;

    // layout
    QMargins margins = QMargins(10,10,10,10);
    qreal lineSpacing = 5.0f;

    // find
    QHash<int, ChatLine::Ptr> foundText;

    // global date
    ChatMessage::Ptr globalDateMessage;
    QGraphicsRectItem* globalDateRect;
    QVector<QPair<QDate, ChatMessage::Ptr>> dateMessages;
    int globalDateIndex;
};

#endif // CHATLOG_H
