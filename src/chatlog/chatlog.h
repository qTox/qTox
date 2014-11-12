#ifndef CHATLOG_H
#define CHATLOG_H

#include <QGraphicsView>
#include <QDateTime>

class QGraphicsScene;
class ChatLine;
class ChatLineContent;

class ChatLog : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ChatLog(QWidget* parent = 0);
    virtual ~ChatLog();

    void addTextLine(const QString& sender, const QString& text, QDateTime timestamp = QDateTime::currentDateTime());
    void addWidgetLine(const QString& sender, QDateTime timestamp = QDateTime::currentDateTime());
    void insertChatline(ChatLine* l);

    void clearSelection();
    void clear();
    void copySelectedText() const;
    QString getSelectedText() const;

    void dbgPopulate();

signals:

public slots:

protected:
    QRect getVisibleRect() const;
    ChatLineContent* getContentFromPos(QPointF scenePos) const;

    bool layout(int start, int end, qreal width);
    bool isOverSelection(QPointF scenePos);
    bool stickToBottom();

    int useableWidth();

    void reposition(int start, int end);
    void repositionDownTo(int start, qreal end);
    void updateSceneRect();
    void partialUpdate();
    void fullUpdate();
    void checkVisibility();
    void scrollToBottom();
    void showContextMenu(const QPoint& globalPos, const QPointF& scenePos);

    virtual void mousePressEvent(QMouseEvent* ev);
    virtual void mouseReleaseEvent(QMouseEvent* ev);
    virtual void mouseMoveEvent(QMouseEvent* ev);
    virtual void scrollContentsBy(int dx, int dy);
    virtual void resizeEvent(QResizeEvent *ev);

private:
    QGraphicsScene* scene = nullptr;
    QList<ChatLine*> lines;
    QList<ChatLine*> visibleLines;

    bool multiLineInsert = false;
    bool stickToBtm = false;
    int insertStartIndex = -1;

    // selection
    int selStartRow = -1;
    int selStartCol = -1;
    int selLastRow = -1;
    int selLastCol = -1;
    bool selecting = false;
    QPointF clickPos;
    QPointF lastPos;

    // actions
    QAction* copyAction = nullptr;

    // layout
    qreal lineSpacing = 10.0f;

};

#endif // CHATLOG_H
