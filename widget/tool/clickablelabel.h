#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>

class QMouseEvent;
class QWidget;

class ClickableLabel : public QLabel
{
Q_OBJECT

public:
    explicit ClickableLabel(QWidget* parent = 0) : QLabel(parent) {}
signals:
    void clicked();
protected:
    void mousePressEvent (QMouseEvent*) {emit clicked();}
};


#endif // CLICKABLELABEL_H
