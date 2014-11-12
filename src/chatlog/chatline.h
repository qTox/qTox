#ifndef CHATLINE_H
#define CHATLINE_H

#include <QTextLayout>

class ChatLog;
class ChatLineContent;
class QGraphicsScene;
class QStyleOptionGraphicsItem;

struct ColumnFormat
{
    enum Policy {
        FixedSize,
        VariableSize,
    };

    enum Align {
        Left,
        Center,
        Right,
    };

    ColumnFormat() {}
    ColumnFormat(qreal s, Policy p, int valign = -1, Align halign = Left)
        : size(s)
        , policy(p)
        , vAlignCol(valign)
        , hAlign(halign)
    {}

    qreal size = 1.0;
    Policy policy = VariableSize;
    int vAlignCol = -1;
    Align hAlign = Left;
};

using ColumnFormats = QVector<ColumnFormat>;

class ChatLine
{
public:
    explicit ChatLine(QGraphicsScene* scene);
    virtual ~ChatLine();

    virtual QRectF boundingSceneRect() const;

    void addColumn(ChatLineContent* item, ColumnFormat fmt);

    void layout(qreal width, QPointF scenePos);
    void layout(QPointF scenePos);

    void selectionCleared();
    void selectionCleared(int col);
    void selectAll();
    void selectAll(int col);

    int getColumnCount();
    int getRowIndex() const;

    bool isOverSelection(QPointF scenePos);

private:
    QPointF mapToContent(ChatLineContent* c, QPointF pos);
    void updateBBox();

    friend class ChatLog;
    void setRowIndex(int idx);
    void visibilityChanged(bool visible);

private:
    int rowIndex = -1;
    QGraphicsScene* scene = nullptr;
    QVector<ChatLineContent*> content; // 3 columns
    QVector<ColumnFormat> format;
    qreal width;
    QRectF bbox;
    QPointF pos;
    bool isVisible = false;

};

#endif // CHATLINE_H
