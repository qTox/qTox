#ifndef TEXT_H
#define TEXT_H

#include "../chatlinecontent.h"

#include <QTextDocument>
#include <QTextCursor>

class ChatLine;
class QTextLayout;

class Text : public ChatLineContent
{
public:
    Text(const QString& txt = "", bool enableElide = false);
    virtual ~Text();

    void setText(const QString& txt);

    virtual void setWidth(qreal width);

    virtual void selectionMouseMove(QPointF scenePos);
    virtual void selectionStarted(QPointF scenePos);
    virtual void selectionCleared();
    virtual void selectAll();
    virtual bool isOverSelection(QPointF scenePos) const;
    virtual QString getSelectedText() const;

    virtual QRectF boundingSceneRect() const;
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual void visibilityChanged(bool isVisible);

    virtual qreal firstLineVOffset();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual QString toString() const;

protected:
    // dynamic resource management
    void ensureIntegrity();
    void freeResources();
    QSizeF idealSize();

    int cursorFromPos(QPointF scenePos) const;

private:
    QTextDocument* doc = nullptr;
    QString text;
    QString elidedText;
    QSizeF size;
    bool isVisible = false;
    bool elide = false;
    QTextCursor cursor;
    qreal vOffset = 0.0;
    qreal width = 0.0;

};

#endif // TEXT_H
