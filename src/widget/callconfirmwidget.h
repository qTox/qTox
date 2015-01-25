#ifndef CALLCONFIRMWIDGET_H
#define CALLCONFIRMWIDGET_H

#include <QWidget>

class QMoveEvent;

class CallConfirmWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CallConfirmWidget(QWidget *anchor);

signals:
    void accepted();
    void rejected();
};

#endif // CALLCONFIRMWIDGET_H
